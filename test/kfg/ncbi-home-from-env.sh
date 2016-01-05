SILENT=-q

################################################################################
# Test preconditions:
#  vdb-config should be in the PATH;
#  cwd should be ncbi-vdb/test/kfg
# ~/.ncbi/user-settings.mkfg should not exist.
#  NCBI_SETTINGS should not be set;
#  VDB_CONFIG should not be set.

N=$HOME/.ncbi
U=$N/user-settings.mkfg
K=$N/test-ncbi-home-from-env.kfg
Z=$N/test-not-user-settings.xml

if [ -e $U ] ; then
    echo $U exists. The test is aborted
    exit 1
fi
if [ ! -e $N ] ; then
    mkdir $N
    DOT_NCBI_CREATED=1
fi

which vdb-config > /dev/null 2>&1
if [ $? != 0 ] ; then
    echo ERROR: cannot find vdb-config. The test is aborted
    exit 1
fi

if [ ! -d ncbi-home-from-env.configs/dir1 ] ; then
    echo ERROR: cannot find ncbi-home-from-env.configs/. The test is aborted
    exit 1
fi

echo $NCBI_SETTINGS | grep $SILENT '^$'
if [ $? != 0 ] ; then
    echo ERROR: NCBI_SETTINGS is set
    exit 1
fi

echo $VDB_CONFIG | grep $SILENT '^$'
if [ $? != 0 ] ; then
    echo ERROR: VDB_CONFIG is set
    exit 1
fi

echo "Info: creating $Z."
echo '/refseq/volumes = "STD-OLD-KFG"' > $Z
echo '/ncbi-home-from-env = "STD-NEW-KFG"' >> $Z

# TESTS ########################################################################

echo Test: no customization.
vdb-config -on | grep $SILENT '^/ncbi-home-from-env'
if [ $? == 0 ] ; then
    echo ERROR: ncbi-home-from-env node was found
    EXIT=1
fi
echo

################################################################################

echo "Info: creating $K."
echo '/refseq/volumes = "STD-OLD-FILE"' > $K
echo '/ncbi-home-from-env = "STD-NEW-FILE"' >> $K

echo Test: load node from kfg file from ~/.ncbi.
vdb-config -on | grep $SILENT '^/refseq/volumes = "STD-OLD-FILE"'
if [ $? != 0 ] ; then
    echo ERROR: cannot load configuration from file from ~/.ncbi
    EXIT=1
fi

echo Test: load new node from kfg file from ~/.ncbi.
vdb-config -on | grep $SILENT '^/ncbi-home-from-env = "STD-NEW-FILE"$'
if [ $? != 0 ] ; then
    echo ERROR: cannot load new configuration from file from ~/.ncbi
    EXIT=1
fi
echo

################################################################################

echo "Info: creating $U."
echo '/refseq/volumes = "STD-OLD-USER"' > $U
echo '/ncbi-home-from-env = "STD-NEW-USER"' >> $U

echo Test: load node from ~/.ncbi/user-settings.mkfg.
vdb-config -on | grep $SILENT '^/refseq/volumes = "STD-OLD-USER"'
if [ $? != 0 ] ; then
    echo ERROR: cannot load configuration from ~/.ncbi/user-settings.mkfg
    EXIT=1
fi
echo Test: load new node from ~/.ncbi/user-settings.mkfg.
vdb-config -on | grep $SILENT '^/ncbi-home-from-env = "STD-NEW-USER"$'
if [ $? != 0 ] ; then
    echo ERROR: cannot load configuration from ~/.ncbi/user-settings.mkfg
    EXIT=1
fi
echo

################################################################################

echo Info: NCBI_SETTINGS=$Z.
export NCBI_SETTINGS=$Z
echo Test: load node from NCBI_SETTINGS.
vdb-config -on | grep $SILENT '^/refseq/volumes = "STD-OLD-KFG"'
if [ $? != 0 ] ; then
    echo ERROR: cannot load configuration from NCBI_SETTINGS
    EXIT=1
fi
echo Test: load new node from NCBI_SETTINGS.
vdb-config -on | grep $SILENT '^/ncbi-home-from-env = "STD-NEW-KFG"'
if [ $? != 0 ] ; then
    echo ERROR: cannot load new configuration from NCBI_SETTINGS
    EXIT=1
fi
unset NCBI_SETTINGS
echo

################################################################################

echo Info: NCBI_HOME=dir1.
export NCBI_HOME=`pwd`/ncbi-home-from-env.configs/dir1
echo Test: load node from NCBI_HOME.
vdb-config -on | grep $SILENT '^/refseq/volumes = "ONE-OLD-USER"'
if [ $? != 0 ] ; then
    echo ERROR: cannot load configuration from NCBI_HOME
    EXIT=1
fi
echo Test: load new node from NCBI_HOME.
vdb-config -on | grep $SILENT '^/ncbi-home-from-env = "ONE-NEW-USER"'
if [ $? != 0 ] ; then
    echo ERROR: cannot load new configuration from NCBI_HOME
    EXIT=1
fi
echo

################################################################################

echo Info: NCBI_HOME=dir1 NCBI_SETTINGS=dir2/not-user-settings.xml.
export NCBI_SETTINGS=`pwd`/ncbi-home-from-env.configs/dir2/not-user-settings.xml
echo Test: load node from NCBI_HOME/NCBI_SETTINGS.
vdb-config -on | grep $SILENT '^/refseq/volumes = "TWO-OLD-KFG"'
if [ $? != 0 ] ; then
    echo ERROR: cannot load configuration from NCBI_HOME/NCBI_SETTINGS
    EXIT=1
fi
echo Test: load new node from NCBI_HOME/NCBI_SETTINGS.
vdb-config -on | grep $SILENT '^/ncbi-home-from-env = "TWO-NEW-KFG"'
if [ $? != 0 ] ; then
    echo ERROR: cannot load new configuration from NCBI_HOME/NCBI_SETTINGS
    EXIT=1
fi
echo

################################################################################

rm -v $K $U $Z
if [ "$DOT_NCBI_CREATED" != "" ] ; then
    rmdir -v $N
fi

echo
if [ "$EXIT" == '' ] ; then
    echo SUCCESS
else
    echo FAILURE
fi

exit $EXIT

################################################################################
