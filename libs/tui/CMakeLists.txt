if( "linux" STREQUAL ${OS} )
    #include_directories(linux)
    set (OS_SPECIFIC_SRC linux/systui.c)
elseif ( "unix" STREQUAL ${OS} )
    #include_directories(unix)
    set (OS_SPECIFIC_SRC unix/systui.c)
elseif ( "win" STREQUAL ${OS} )
    #include_directories(win)
    set (OS_SPECIFIC_SRC win/systui.c)
endif()

add_library(tui STATIC
    dir_dlg.c
    eventring.c
    file_dlg.c
    line_policy.c
    screen.c
    string_cache.c
    tui.c
    tui_cpp.cpp
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

add_definitions (-D__mod__="libs/tui")