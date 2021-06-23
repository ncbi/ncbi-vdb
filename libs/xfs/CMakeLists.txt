if( "unix" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC unix/operations.c unix/platform.c unix/security.c)
elseif ( "win" STREQUAL ${OS} )
    set (OS_SPECIFIC_SRC win/operations.c win/platform.c win/security.c)
endif()

add_library(xfs STATIC
    bogus.c
    contnode.c
    doc.c
    docnode.c
    editors.c
    encro.c
    encws.c
    foo.c
    gap.c
    gapc.c
    gapf.c
    gapk.c
    gapka.c
    gapks.c
    handle.c
    http.c
    kfs.c
    lockdpt.c
    lreader.c
    mehr.c
    model.c
    ncon.c
    node.c
    owp.c
    path.c
    perm.c
    readme.c
    schwarzschraube.c
    simcon.c
    spen.c
    tar.c
    teleport.c
    tree.c
    xencro.c
    xencws.c
    xfs.c
    xgap.c
    xgapf.c
    xgapk.c
    xhttp.c
    xlog.c
    xtar.c
    zehr.c
    ${OS_SPECIFIC_SRC}
)

add_definitions (-D__mod__="libs/xfs")