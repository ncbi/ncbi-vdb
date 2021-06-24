set ( UNIX_SRC unix/systui.c )
set ( LINUX_SRC linux/systui.c )
set ( MAC_SRC ${UNIX_SRC} )
set ( WIN_SRC win/systui.c )

if ( "win" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
    #include_directories( win )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
    #include_directories( unix )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
    #include_directories( unix )
endif()

add_library( tui STATIC
    dir_dlg.c
    eventring.c
    file_dlg.c
    line_policy.c
    screen.c
    string_cache.c
    tui.c
    tui_dlg.c
    tui_dlg_helper.c
    tui_menu.c
    tui_palette.c
    tui_std_dlg.c
    tui_widget.c
    tui_widget_button.c
    tui_widget_checkbox.c
    tui_widget_grid.c
    tui_widget_inputline.c
    tui_widget_label.c
    tui_widget_progress.c
    tui_widget_radiobox.c
    tui_widget_spin_edit.c
    tui_widget_string_list.c
    tui_widget_tabhdr.c

    ${OS_SPECIFIC_SRC}
)
add_library( tui_cpp STATIC tui_cpp )

add_definitions (-D__mod__="libs/tui")