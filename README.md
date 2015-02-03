# The NCBI SRA (Sequence Read Archive)

### Contact:
email: sra-tools@ncbi.nlm.nih.gov
or visit the [SRA Tools web site](http://www.ncbi.nlm.nih.gov/Traces/sra/?view=toolkit_doc)

## Release 2.4.4
This release contains improvements in the area of networking. A bug in our
networking code was introduced recently that allowed HTTP server connection
resets to go undetected, which manifested itself as a "corrupt blob" error.
What was happening was that the download-on-demand code was properly detecting a
dropped connection, but failing to report it to consuming code. The latter in
turn saw blobs of invalid data, and correctly reported that they were corrupt.

The most common manifestation of this problem was with fastq-dump, which seemed
to encounter a corrupt blob error after dumping some number of reads. The
reported problem was correct. However, the source of the error was not a corrupt
download, but instead was the result of dropped connections with the HTTP
server.

This release corrects the problem with unreported cases of dropped connections,
and so restores the expected behavior.

## NCBI-VDB
The NCBI-VDB library implements a highly compressed columnar data warehousing
engine that is most often used to store genetic information. Databases are
stored in a portable image within the file system, and can be accessed/downloaded
on demand across HTTP.

For additional information on using, configuring, and building the library,
please visit our wiki: https://github.com/ncbi/ncbi-vdb/wiki.


SRA Toolkit Development Team
