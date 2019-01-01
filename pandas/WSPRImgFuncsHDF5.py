import pandas as pd
from pandas import HDFStore
import numpy as np
import subprocess
import io
import matplotlib.pyplot as plt
import gc
import os

from functools import lru_cache

'''
 Analyze wsprspots logs (prepared by WSPRLog2Pandas)
 All manipulations are performed against an HDF5 store
 with all reports in the "norm" dataset and the image
 reports in the 'img' dataset.
'''

# utility functions
def absRangeMask(ser, min, max):
    return (abs(ser) < min) | (abs(ser) > max)

def findrange(v):
    for expd in range(-16,16):
        for mant in [1, 2, 5]:
            lim = mant * (10 ** expd)
            #print("test v = %g  lim = %g\n" % (v, lim))
            if v < lim:
                return lim
    return v

# numeric filter functions
def identityFunc(v):
    return v

def roundAZ(val, min, interval):
    rv = roundInterval(val, min, interval)
    rv[rv == 360] = 0
    return rv
    
def roundInterval(val, min, interval):
    '''
    Round val to the nearest value that is min + N*interval
    '''
    Nf = (val - min)/interval
    N = np.round(Nf)
    return min + N * interval

def truncInterval(val, min, interval):
    Nf = (val - min)/interval
    N = np.trunc(Nf)
    return min + N * interval

@lru_cache(maxsize=64)
def lamInterval(func, min, interval):
    return lambda x : func(x, min, interval)


class WSPRImg:
    def __init__(self, store_name, file_list=None, exp_name='', use_hdf5_store=False):
        # init these sets as empty -- they're used as filters
        # in the get chunks method. 
        self.bad_rx_calls = set()
        self.bad_txrx_pairs = set()
        self.exp_name = exp_name;

        if (file_list == None) || (use_hdf5_store && os.path.isfile(store_name)):
            print("Using HDF5 Store")
            self.store = pd.HDFStore(store_name, mode='r', complib='zlib', complevel=9)
            print("Got here to build exc list")
            self.buildExclusionLists()
        else:
            self.readFilesToHDF5(file_list, store_name)
            self.store.info
            

    def __del__(self):
        self.store.close()
        del self.bad_rx_calls
        del self.bad_txrx_pairs
        gc.collect()
        
    def readFilesToHDF5(self, flist, store_name):
        '''
        Read a list of files into an HDF5 store. 
        Read the input in chunks to keep storage to a minimum. '

        The store will be written to two datasets in the hdf5 file. 
        'norm' is the set of all reports, and 'img' is the set of all
        image reports. 

        On the way in, we create a new column "TXRX" with the catenation 
        of the TXCALL and RXCALL values. 
        '''
        col_types = {'RXSOL':float, 'TXSOL':float, 'MIDSOL':float, 
                     'SPOT':int, 'DTIME':int, 'DIFFSNR':float, 
                     'RXCALL':str, 'TXCALL':str, 'RXGRID':str, 'TXGRID':str, 
                     'REFSNR':float, 'FREQ':float, 'POW':float, 'DRIFT':float, 
                     'DIST':float, 'AZ':float, 'BAND':str, 'VER':str, 'CODE':int, 
                     'FREQDIFF':float}

        item_sizes = {'RXCALL': 12, 'TXCALL':12, 'RXGRID':8, 'TXGRID':8, 
                      # 'VER': 20, # 'BAND': 20, # 'CODE': 20,
                      'TXRX':24}
        
        col_names = list(col_types.keys())
        col_names.append('TXRX')
        
    
        csize = 1024 * 1024 # read 1M records at a time.
        # create the datastore
        #self.store = pd.HDFStore(store_name, complib='zlib', complevel=9, columns=col_names, format='table')
        # we accumulate bad lists along the way, and delete suspect contacts
        #self.store = pd.HDFStore(store_name, mode='w', complib='zlib', complevel=9, format='table')
        #self.store = pd.HDFStore(store_name, mode='w', complib='blosc:lz4', complevel=9, format='table')
        self.store = pd.HDFStore(store_name, mode='w', complib='bzip2', complevel=9, format='table')        
        rcount = 0
        for fn in flist:
            for chunk in pd.read_csv(fn, dtype=col_types, chunksize=csize):
                chunk['TXRX'] = chunk['TXCALL'].str.cat(chunk['RXCALL'])
                
                chunk = chunk.drop(['BAND','VER','CODE','SPOT'],axis=1)

                # create the image frame
                img_chunk = self.filterLineFreqs(self.getImageFrame(chunk))

                # now accumulate the bad calls
                self.buildExclusionListOTF(img_chunk)

                for col in ['RXSOL','TXSOL','MIDSOL','DIFFSNR','REFSNR','POW','DRIFT','AZ','FREQDIFF']:
                    chunk[col] = chunk[col].astype(np.float32)

                # remove them from the chunk
                chunk2 = chunk[~(chunk.RXCALL.isin(self.bad_rx_calls) | chunk.TXRX.isin(self.bad_txrx_pairs))]

                    
                # save all reports in the norm table
                self.store.append('norm', chunk2, data_columns=True, min_itemsize = item_sizes)
                # save image reports in the image table
                self.store.append('img', self.filterLineFreqs(self.getImageFrame(chunk2)), 
                                  data_columns=True, min_itemsize = item_sizes)
                del chunk
                del chunk2
                del img_chunk
                
                print("%d\n" % rcount, end='')
                rcount = rcount + csize
                gc.collect()
                
        return

    def getImageFrame(self, fr):
        return fr[fr.FREQDIFF != 0]
        
    def filterLineFreqs(self, fr):
        # return an all true mask
        msk = fr.FREQDIFF < 1e19
        for min,max in ((58,62), (48,52)):
            for mul in (1, 2, 3):
                lmin = min * mul
                lmax = max * mul
                nmsk = absRangeMask(fr.FREQDIFF, lmin, lmax)
                msk = msk & nmsk
        return fr[msk]

    def buildExclusionListOTF(self, chunk):
        '''
        Build the exclusion list as we read it. (on the fly)
        '''
        # build the series from the value counts in a chunk
        # then turn the series into a set (s = set(ser.unique())
        # isin can test against a set (!)
        # merge the sets as s1.union(s2) (does not modify either set...) or s1 | s2
        tmp_rx_counts = chunk['RXCALL'].value_counts()
        tmp_txrx_counts = chunk['TXRX'].value_counts()

        srx = set(tmp_rx_counts[tmp_rx_counts > 4].index.to_series().unique())
        stxrx = set(tmp_txrx_counts[tmp_txrx_counts > 3].index.to_series().unique())
        
        self.bad_rx_calls = self.bad_rx_calls.union(srx)
        self.bad_txrx_pairs = self.bad_txrx_pairs.union(stxrx)
        print("bad_rx_calls len = %d bad_tx_pairs len = %d\n" % (len(self.bad_rx_calls), len(self.bad_txrx_pairs)))
        
        
    def buildExclusionLists(self):
        '''
        When we process blocks of records, we need to skip records that
        may be suspect in origin. These include records from RX stations
        that report too many image events, and TX/RX pairs that report more
        than 5 events.  

        These lists are called bad_rx_calls and bad_txrx_pairs
        '''
        # iterate through all the records in blocks of 100K rows
        # we only need to scan the image list.
        tmp_rx_counts = pd.Series([])
        tmp_txrx_counts = pd.Series([])

        chunksize = 100000
        nrows = self.getDataSetSize('img')

        for beg in range(0, nrows, chunksize):
            chunk = self.getChunk('img', beg, chunksize);
            tmp_rx_counts = tmp_rx_counts.append(chunk['RXCALL'].value_counts())
            tmp_txrx_counts = tmp_txrx_counts.append(chunk['TXRX'].value_counts())

        tmp_rx_counts = tmp_rx_counts.groupby(tmp_rx_counts.index).sum()
        tmp_txrx_counts = tmp_txrx_counts.groupby(tmp_txrx_counts.index).sum()

        self.bad_rx_calls = set(tmp_rx_counts[tmp_rx_counts > 4].index.to_series().unique())
        self.bad_txrx_pairs = set(tmp_txrx_counts[tmp_txrx_counts > 3].index.to_series().unique())

    def getDataSetSize(self, data_set_name):
        return self.store.get_storer(data_set_name).nrows

    @lru_cache(maxsize=16)
    def getChunk(self, data_set_name, beg, length):
        '''
        Get a chunk of (data_set)from the HDF store, filter out reports with the bad lists.
        '''
        chunk = self.store.select(key=data_set_name, start=beg, stop=beg+length-1)
        return chunk[~(chunk.RXCALL.isin(self.bad_rx_calls) | chunk.TXRX.isin(self.bad_txrx_pairs))]

    @lru_cache(maxsize=16)    
    def storeValueCount(self, data_set_name, column, binfunc = identityFunc, sort=True):
        '''
        Read the store in chunks and do a value count. 
        We could do this all in one swoop, but it will allocate wayyyy too much memory 
        for large datasets. 
        
        So we read this in chunks as per a useful stack overflow suggestion
        https://stackoverflow.com/questions/26019834/value-counts-of-a-database-chunk-by-chunk-using-pandas
        
        For tables where the index is a category, the bin function should return its sole argument. 
        '''
        nrows = self.getDataSetSize(data_set_name)
        chunksize = 500000
        temp = pd.Series([])

        for beg in range(0, nrows, chunksize):
            chunk = self.getChunk(data_set_name, beg, chunksize) #self.store.select(key=data_set_name, start=beg, stop=beg+chunksize-1)
            temp = temp.append(binfunc(chunk[column]).value_counts())
            print("%d\n" % beg, end='')
            gc.collect()

        # now we've got appended value counts... sum the common columns
        res = temp.groupby(temp.index).sum()

        # return the result        
        if sort:
            return res.sort_values(ascending=False)
        else:
            return res


    def genGraphCommon(self, colname, binfunc, xlim, xticks, xlabel, ylabel, title, ylim, comptype):
        '''
        Common graph operations for all plot generation. 

        Data can be either conditional (probability of an X given a Y with column value C 
        comptype = COMP)
        or PDF of X vs. column value C in dataset D  (comptype is IMG or NORM)
        '''
        fig, ax = plt.subplots()
        ax.set_xlabel(xlabel)
        ax.set_ylabel(ylabel)
        ax.set_xticks(xticks);
        
        
        if xlim != '':
            ax.set_xlim(xlim)

        if title == '':            
            if comptype == 'COND':
                title = 'Probability a Report is an Image vs. %s (dataset %s)' % (colname, self.exp_name)
            elif comptype == 'NORM':
                title = 'Probability of a \"Normal\" Report vs. %s (dataset %s)' % (colname, self.exp_name)
            elif comptype == 'IMG':
                title = 'Probability of an \"Image\" Report vs. %s (dataset %s)' % (colname, self.exp_name)        
        ax.set_title(title);

        vec = self.genVecNumeric(colname, binfunc, True, comptype)

        yrange = findrange(vec.max(0))
        print("vec max = %g yrange = %g\n" % (vec.max(0), yrange))
        
        if ylim != '':
            ax.set_ylim(ylim)
        else:
            ax.set_ylim((0, yrange))
            print("set ylim here vec max = %g yrange = %g\n" % (vec.max(0), yrange))        

        return (fig, ax, vec)
        
    def genPlotNumeric(self, colname, binfunc=np.trunc,
                       xlim=(-1000,1000), xticks=np.arange(-1000,1000,500), 
                       xlabel='', ylabel='', title='', ylim='', comptype='COND'):
        fig, ax, vec = self.genGraphCommon(colname, binfunc, xlim, xticks, xlabel, ylabel, title, ylim, comptype)
        #vec.plot(grid=True, xticks=xticks, xlim=xlim, ax=ax)
        vec.plot(grid=True, ax=ax)        
        return fig


    def genScatterNumeric(self, colname, binfunc=np.trunc,
                          xlim=(-1000,1000), xticks=np.arange(-1000,1000,500), 
                          xlabel='', ylabel='', title='', ylim='', comptype='COND'):
        fig, ax, vec = self.genGraphCommon(colname, binfunc, xlim, xticks, xlabel, ylabel, title, ylim, comptype)
        print("genScatterNumeric: vec max = %g\n" % vec.max(0))
        print(vec.shape)
        ax.scatter(vec.index, vec.values) # xticks=xticks, xlim=xlim)
        ax.grid()
        return fig

    @lru_cache(maxsize=128)
    def genVecNumeric(self, colname, binfunc, fill=True, comptype='COND'):
        '''
        Compute the probability of an exceptional observation matching 'colname' given
        the occurance of a normal observation. 
        
        For instance genVecNumeric(farm_animals, goats, 'weight_in_pounds', np.rint) will return a vector K
        such that K[x] is the probability that an animal is a goat given that the weight is X pounds.
        '''
        res = pd.Series([])
        
        if comptype == 'COND':
            f1_hv = self.storeValueCount('norm', colname, binfunc, sort=False)
            f2_hv = self.storeValueCount('img', colname, binfunc, sort=False)
            res = f2_hv / f1_hv
            if fill:
                res.fillna(0, inplace=True)
        else:
            if comptype == 'NORM':                
                res = self.storeValueCount('norm', colname, binfunc, sort=False)            

            elif comptype == 'IMG':
                res = self.storeValueCount('img', colname, binfunc, sort=False)            

            res = res.sort_index(ascending=True) / res.sum(0)            

        return res

