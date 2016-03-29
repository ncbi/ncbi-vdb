from ctypes import (c_int, c_char_p, cdll, Structure, c_uint32, ARRAY,
                    c_char, POINTER, c_size_t, c_void_p, c_uint64, byref,
                    create_string_buffer, c_uint)
import platform

import os


def check_rc(rc):
    if rc:
        raise RuntimeError("bad rc, %d", rc)
    return 1

def get_lib_name():
    if platform.system() == "Windows":
        return "ncbi-vdb.dll"
    elif platform.system() == "Darwin":
        return "libncbi-vdb.dylib"
    else:
        return "libncbi-vdb.so"


_krypto = _kfs = cdll.LoadLibrary( get_lib_name() )


class _KKey(Structure):
    _fields_ = [("type", c_uint32),
                ("text", ARRAY(c_char, 32))]


def errcheck(result, func, args):
    return check_rc(result)

# setting up declarations for functions
KKeyInitRead = _krypto.KKeyInitRead
KKeyInitRead.restype = c_uint
KKeyInitRead.argtypes = [POINTER(_KKey), c_int, c_char_p, c_size_t]
KKeyInitRead.errcheck = errcheck

KEncFileMakeRead = _krypto.KEncFileMakeRead
KEncFileMakeRead.restype = c_uint
KEncFileMakeRead.argtypes = (POINTER(c_void_p), c_void_p, POINTER(_KKey))
KEncFileMakeRead.errcheck = errcheck

KEncryptFileMakeRead = _krypto.KEncryptFileMakeRead
KEncryptFileMakeRead.restype = c_uint
KEncryptFileMakeRead.argtypes = (POINTER(c_void_p), c_void_p, POINTER(_KKey))
KEncryptFileMakeRead.errcheck = errcheck

KEncFileMakeUpdate = _krypto.KEncFileMakeUpdate
KEncFileMakeUpdate.restype = c_uint
KEncFileMakeUpdate.argtypes = (POINTER(c_void_p), c_void_p, POINTER(_KKey))
KEncFileMakeUpdate.errcheck = errcheck

KEncFileMakeWrite = _krypto.KEncFileMakeWrite
KEncFileMakeWrite.restype = c_uint
KEncFileMakeWrite.argtypes = (POINTER(c_void_p), c_void_p, POINTER(_KKey))
KEncFileMakeWrite.errcheck = errcheck

KReencFileMakeRead = _krypto.KReencFileMakeRead
KReencFileMakeRead.restype = c_uint
KReencFileMakeRead.argtypes = [POINTER(c_void_p), c_void_p, POINTER(_KKey), POINTER(_KKey)]
KReencFileMakeRead.errcheck = errcheck

KFileMakeFDFileRead = _kfs.KFileMakeFDFileRead
KFileMakeFDFileRead.restype = c_uint
KFileMakeFDFileRead.argtypes = [POINTER(c_void_p), c_int]
KFileMakeFDFileRead.errcheck = errcheck

KFileMakeFDFileWrite = _kfs.KFileMakeFDFileWrite
KFileMakeFDFileWrite.restype = c_uint
KFileMakeFDFileWrite.argtypes = [POINTER(c_void_p), c_int, c_int]
KFileMakeFDFileWrite.errcheck = errcheck

KFileRelease = _kfs.KFileRelease
KFileRelease.argtypes = [c_void_p]

KFileReadAll = _kfs.KFileReadAll
KFileReadAll.restype = c_uint
KFileReadAll.argtypes = [c_void_p, c_uint64, c_void_p, c_size_t, POINTER(c_size_t)]
KFileReadAll.errcheck = errcheck

KFileWrite = _kfs.KFileWrite
KFileWrite.restype = c_uint
KFileWrite.argtypes = [c_void_p, c_uint64, c_char_p, c_size_t, POINTER(c_size_t)]
KFileWrite.errcheck = errcheck

KFileWriteAll = _kfs.KFileWriteAll
KFileWriteAll.restype = c_uint
KFileWriteAll.argtypes = [c_void_p, c_uint64, c_char_p, c_size_t, POINTER(c_size_t)]
KFileWriteAll.errcheck = errcheck

KFileSize = _kfs.KFileSize
KFileSize.restype = c_uint
KFileSize.argtypes = [c_void_p, POINTER(c_uint64)]
KFileSize.errcheck = errcheck

KFileSetSize = _kfs.KFileSetSize
KFileSetSize.restype = c_uint
KFileSetSize.argtypes = [c_void_p, c_uint64]
KFileSetSize.errcheck = errcheck

_kkeyAES128 = 1

class _KFile(object):
    def __init__(self):
        self._kfile = None

    def __dealloc__(self):
        if self._kfile:
            KFileRelease(self._kfile)

    def __enter__(self):
        return self

    def __exit__(self, t, value, traceback):
        self.close()

    def close(self):
        if self._kfile:
            KFileRelease(self._kfile)
            self._kfile = None

    def write_all(self, buf, offset):
        """
        Write `buf` bytes into file at given `offset`
        :param buf: Byte-string to write.
        :param offset: Offset position in bytes to start writing at
        """
        if not self._kfile:
            raise ValueError("I/O operation on closed file")
        written = c_size_t()
        KFileWriteAll(self._kfile, offset, buf, len(buf), byref(written))
        return written.value

    def read_all(self, size, offset):
        """
        Read `size` bytes from `offset`, might return less than `size` bytes if EOF is reached before.

        :param size: Number of bytes to read
        :param offset: Offset to start read at
        :return: A byte-buffer containing read bytes
        """
        if not self._kfile:
            raise ValueError("I/O operation on closed file")
        read = c_size_t()
        buf = create_string_buffer(size)
        KFileReadAll(self._kfile, offset, buf, size, byref(read))
        # return buf.value[0:read.value]
        return buf[0:read.value]

    @property
    def size(self):
        """ Size of the file in bytes
        """
        if not self._kfile:
            raise ValueError("I/O operation on closed file")
        sz = c_uint64()
        KFileSize(self._kfile, byref(sz))
        return sz.value


class _KFileStream(_KFile):
    def __init__(self):
        super(_KFileStream, self).__init__()
        self.pos = 0

    def write(self, buf):
        """ Write `buf` into current position of the file.

        :param buf: Byte-string to write into file
        """
        left = len(buf)
        total_written = 0
        while left:
            written = self.write_all(buf[total_written:], self.pos)
            self.pos += written
            total_written += written
            left -= written
        return total_written

    def seek(self, pos):
        """ Sets current position in the file """
        self.pos = pos

    def read(self, size=-1):
        """ Read `size` bytes from file
        """
        strings = []
        if not self._kfile:
            raise ValueError("I/O operation on closed file")
        if size == -1:
            strings = []
            while 1:
                s = self.read_all(16 * 1024, self.pos)
                self.pos += len(s)
                if not s:
                    break
                strings.append(s)
            return b''.join(strings)
        else:
            res = self.read_all(size, self.pos)
            self.pos += len(res)
            return res

    def truncate(self, size=None):
        """
        Truncate file up to current position (default) or up to `size`
        """
        if size is None:
            size = self.pos
        KFileSetSize(self._kfile, size)


class Decryptor(_KFileStream):
    """
    Create a read-only file-like object that will decrypt `src_file` using `password`.

    :param password: Byte string with password for decryption.
    :param src_file: Integer file descriptor of input file.
    :param close: If true `src_file` will be closed when this object is closed.
    """

    def __dealloc__(self):
        if self.src_file:
            os.close(self.src_file)

    def __init__(self, password, src_file, close=True):
        super(Decryptor, self).__init__()
        self.kencfile = c_void_p()
        self.src_file = src_file
        self._close = close
        key = _KKey()
        kfile = c_void_p()
        KKeyInitRead(byref(key), _kkeyAES128, password, len(password))
        KFileMakeFDFileRead(byref(kfile), src_file)
        kencfile = c_void_p()
        try:
            KEncFileMakeRead(byref(kencfile), kfile, byref(key))
        except:
            KFileRelease(kfile)
            raise
        self._kfile = kencfile

    def close(self):
        """
        Releases memory and closes `src_file` if was requested so in constructor.
        """
        super(Decryptor, self).close()
        if self.src_file and self._close:
            os.close(self.src_file)
            self.src_file = None


class Encryptor(_KFileStream):
    """
    Create a file-like object that will encrypt input data and write it into `out_file` file descriptor.

    Call `write` method on this object to write encrypted data into out_file.

    :param password: Byte string with password.
    :param out_file: Integer file descriptor of output file.
    :param close: If true `out_file` descriptor will be closed when this object is closed.
    :param update: If true encrypted file can be appended/modified, otherwise it will be truncated.
    """

    def __dealloc__(self):
        if self.src_file and self._close:
            os.close(self.src_file)

    def __init__(self, password, out_file, close=True, update=False):
        super(Encryptor, self).__init__()
        kfile = c_void_p()
        self.key = _KKey()
        self.out_file = out_file
        kencfile = c_void_p()
        self._close = close
        KKeyInitRead(byref(self.key), _kkeyAES128, password, len(password))
        KFileMakeFDFileWrite(byref(kfile), 1 if update else 0, out_file)
        try:
            if update:
                KEncFileMakeUpdate(byref(kencfile), kfile, byref(self.key))
            else:
                KEncFileMakeWrite(byref(kencfile), kfile, byref(self.key))
        except:
            KFileRelease(kfile)
            raise
        self._kfile = kencfile

    def close(self):
        """ Releases memory and closes `out_file` if was requested so in constructor.
        """
        super(Encryptor, self).close()
        if self.out_file and self._close:
            os.close(self.out_file)
            self.out_file = None

    def flush(self):
        # TODO: implement this
        pass


class Reencryptor(_KFileStream):
    """
    Create a read-only file-like object that will decrypt `src_file` using `dec_password` and then
    encrypt it with `enc_password`.  This objects acts like a decryptor attached to encryptor.

    :param dec_password: Byte-string of decryption password used to decrypt `src_file`.
    :param enc_password: Byte-string of encryption password used to encrypt returned data.
    :param src_file: An integer file descriptor of input file, it should have read operation allowed.
    :param close: If true `src_file` descriptor will be closed when this object is closed.
    """

    def __init__(self, dec_password, enc_password, src_file, close=True):
        super(Reencryptor, self).__init__()
        kfile = c_void_p()
        dec_key = _KKey()
        enc_key = _KKey()
        self.src_file = src_file
        kencfile = c_void_p()
        self._close = close
        KKeyInitRead(byref(dec_key), _kkeyAES128, dec_password, len(dec_password))
        KKeyInitRead(byref(enc_key), _kkeyAES128, enc_password, len(enc_password))
        KFileMakeFDFileRead(byref(kfile), src_file)
        try:
            KReencFileMakeRead(byref(kencfile), kfile, byref(dec_key), byref(enc_key))
        except:
            KFileRelease(kfile)
            raise
        self._kfile = kencfile

    def __dealloc__(self):
        if self.src_file and self._close:
            os.close(self.src_file)

    def close(self):
        """
        Releases memory and closes `src_file` if was requested so in constructor
        """
        super(Reencryptor, self).close()
        if self.src_file and self._close:
            os.close(self.src_file)
            self.src_file = None


class FDFileReader(_KFileStream):
    """
    Deprecated, do not use.  Will be removed in 1.6
    Create a read-only file-like object that will read `f` file-object

    :param f: Source file-like object, should have `fileno` method.
    :param close: If true will close `f` when this object is closed
    """

    def __init__(self, f, close=True):
        super(FDFileReader, self).__init__()
        self._f = f
        kfile = c_void_p()
        KFileMakeFDFileRead(byref(kfile), f.fileno())
        self._kfile = kfile
        self._close = close

    def close(self):
        super(FDFileReader, self).close()
        if self._close:
            self._f.close()


class EncryptingReader(_KFileStream):
    """
    Create a read-only file-like object that will read and encrypt `src_file` using `password`

    :param src_file: An integer file descriptor to be encrypted.
    :param password: Byte-string password used to encrypt.
    :param close: If true `src_file` descriptor will be closed when this object is closed
    """

    def __init__(self, src_file, password, close=True):
        enc_key = _KKey()
        KKeyInitRead(byref(enc_key), _kkeyAES128, password, len(password))
        kencfile = c_void_p()
        KEncryptFileMakeRead(byref(kencfile), src_file._kfile, byref(enc_key))
        self._kfile = kencfile
        self._close = close
        self._src_file = src_file

    def close(self):
        """
        Releases memory and closes `src_file` if it was requested so in constructor
        """
        super(EncryptingReader, self).close()
        if self._close:
            self._src_file.close()
