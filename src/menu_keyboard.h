/* Keyboard requests belong to the window thread, even with no pad polls. */
static void ui_keyboard_actions(void) {
    if(g_ui_bind_mode) {
        int key=g_ui_req_bound_key;g_ui_req_bound_key=0;
        if(key)ui_hotkey_apply(key);
        return;
    }
    if(g_ui_req_toggle) {
        g_ui_req_toggle=0;
        if(g_ui_open && g_ui_page) {g_ui_sel=g_ui_page==3?4:g_ui_page==2?9:0;g_ui_page=0;}
        else {g_ui_open=!g_ui_open;g_ui_page=0;g_ui_sel=0;}
        g_ui_dirty=1;if(g_ui_open)log_text("menu opened (keyboard; controller independent)");
    }
    if(!g_ui_open)return;
    int cards=g_ui_req_cards;g_ui_req_cards=0;
    if(cards){g_ui_controls_page=sc_controls_next_page(g_ui_controls_page,cards,UI_NCARDS,UI_CARDS_PER_PAGE);g_ui_dirty=1;}
    int nav=g_ui_req_nav;g_ui_req_nav=0;
    if(nav){g_ui_overwrite=0;g_ui_preset_status=L"";g_ui_sel=(g_ui_sel+nav+ui_row_count())%ui_row_count();g_ui_dirty=1;}
    int change=g_ui_req_change;g_ui_req_change=0;
    if(change)ui_change(ui_item_for_row(g_ui_sel),change);
}
