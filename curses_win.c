#include "curses_win.h"

int zoom_months[] = {60, 48, 36, 24, 12, 9, 6, 3, 1}, zoom_change_x_months[] = {12, 12, 12, 12, 12, 3, 3, 3, 2};

void portfolio_printw(void) {
    String* pString = portfolio_ncurses_get_plaintext_string(NULL);
    if (pString != NULL) {
        Info_Array* portfolio_data = portfolio_info_array_init_from_portfolio_string(
                pString);
        if (portfolio_data != NULL) {
            api_store_info_array(portfolio_data, DATA_LEVEL_CHECK);
            info_array_portfolio_printw(portfolio_data);
            info_array_destroy(&portfolio_data);
        }
        string_destroy(&pString);
    }
}

void info_array_portfolio_printw(Info_Array* portfolio_data) {
    if (portfolio_data == NULL)
        return;

    initscr();
    noecho(); // Don't echo keystrokes
    keypad(stdscr, TRUE); // Enables extra keystrokes
    curs_set(FALSE); // Hides cursor
    start_color();
    init_pair(2, COLOR_WHITE, COLOR_BLACK); // Init black background, white foreground
    bkgd(COLOR_PAIR(2)); // set background/foreground
    refresh();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    if (cols < 110) {
        endwin();
        puts("Terminal too small.");
        info_array_destroy(&portfolio_data);
        return;
    }

    WINDOW* header_window = newwin(1, cols, 0, 0), * list_window, * total_window;
    int scroll_on = portfolio_data->length > (size_t) rows - 3;
    if (scroll_on)
        list_window = newwin(rows - 3, cols, 1, 0);
    else list_window = newwin((int) portfolio_data->length, cols, 1, 0);
    if (scroll_on)
        total_window = newwin(1, cols, rows - 1, 0);
    else total_window = newwin(1, cols, (int) portfolio_data->length + 2, 0);

    int sort_option = SORT_ALPHA; // Defaults to sort alphabetically
    int highlight_index = HIGHLIGHT_NONE; // Defaults to no highlight
    char highlight_sym[SYMBOL_MAX_LENGTH]; // Symbol of highlighted index
    int scroll_index = 0; // Defaults to first index
    int ch = 0; // getch() data from keyboard

    // For printing/formatting categories
    char* sort_categories_str[] = {"SYMBOL", "VALUE", "SPENT", "PROFIT", "(%)", "24H", "(%)", "7D", "(%)", "30D", "(%)"}
    , * sort_spacing_str[] = {"    ", "    ", "   ", "       ", "      ", "       ", "       ", "       ", "      ",
                              "       ", "\n"};
    do {
        /** HEADER **/
        wattron(header_window, A_BOLD); // Bold categories
        mvwprintw(header_window, 0, 0, "  AMOUNT ");
        for (int i = 0; i < SORT_PROFIT_30D_PERCENT + 1; i++) {
            if (sort_option == i) // Highlight current sorting category
                wattron(header_window, A_STANDOUT);
            wprintw(header_window, "%s", sort_categories_str[i]);
            if (sort_option == i)
                wattroff(header_window, A_STANDOUT);
            wprintw(header_window, "%s", sort_spacing_str[i]);
        }
        wattroff(header_window, A_BOLD);
        wrefresh(header_window);

        /** LIST **/
        portfolio_sort(portfolio_data, sort_option); // Sort using sort_option

        Info* info;
        for (int i = scroll_index; i < (int) portfolio_data->length && i < getmaxy(list_window) + scroll_index; i++) {
            if (highlight_index == i) // Bold highlighted index
                wattron(list_window, A_BOLD);
            info = portfolio_data->array[i]; // Print security data one at a time
            mvwprintw(list_window, i - scroll_index, 0, "%8.2lf %6s %8.2lf %8.2lf %8.2lf (%6.2lf%%) %8.2lf (%6.2lf%%) %8.2lf "
                                                        "(%6.2lf%%) %8.2lf (%6.2lf%%)\n",
                      info->amount, info->symbol, info->current_value, info->total_spent, info->profit_total,
                      info->profit_total_percent, info->profit_last_close, info->profit_last_close_percent,
                      info->profit_7d, info->profit_7d_percent, info->profit_30d, info->profit_30d_percent);
            if (highlight_index == i)
                wattroff(list_window, A_BOLD);
        }
        wrefresh(list_window);

        /** TOTALS **/
        wattron(total_window, A_BOLD); // Bold totals
        mvwprintw(total_window, 0, 0,
                  "         TOTALS %8.2lf %8.2lf %8.2lf (%6.2lf%%) %8.2lf (%6.2lf%%) %8.2lf "
                  "(%6.2lf%%) %8.2lf (%6.2lf%%)", portfolio_data->totals->current_value,
                  portfolio_data->totals->total_spent, portfolio_data->totals->profit_total,
                  portfolio_data->totals->profit_total_percent,
                  portfolio_data->totals->profit_last_close,
                  portfolio_data->totals->profit_last_close_percent,
                  portfolio_data->totals->profit_7d, portfolio_data->totals->profit_7d_percent,
                  portfolio_data->totals->profit_30d,
                  portfolio_data->totals->profit_30d_percent);
        wattroff(total_window, A_BOLD);
        wrefresh(total_window);


        /** USER INPUT **/
        ch = getch(); // Get keyboard input

        // If key right and not right-most sort option
        if (ch == KEY_RIGHT && sort_option != SORT_PROFIT_30D_PERCENT) { // key RIGHT -- moves sort category right
            if (highlight_index != HIGHLIGHT_NONE) {
                strcpy(highlight_sym, portfolio_data->array[highlight_index]->symbol);
                sort_option++;
                portfolio_sort(portfolio_data, sort_option);
                for (size_t i = 0; i < portfolio_data->length; i++)
                    if (streq(portfolio_data->array[i]->symbol, highlight_sym))
                        highlight_index = (int) i; // Make sure the same security stays highlighted
            } else sort_option++;

            // If key left and not left-most sort option
        } else if (ch == KEY_LEFT && sort_option != SORT_ALPHA) { // key LEFT -- moves sort category left
            if (highlight_index != HIGHLIGHT_NONE) {
                strcpy(highlight_sym, portfolio_data->array[highlight_index]->symbol);
                sort_option--;
                portfolio_sort(portfolio_data, sort_option); // Sort security array
                for (size_t i = 0; i < portfolio_data->length; i++)
                    if (streq(portfolio_data->array[i]->symbol, highlight_sym))
                        highlight_index = (int) i; // Make sure the same security stays highlighted
            } else sort_option--;

            // If key down and not highlighting last index
        } else if (ch == KEY_DOWN && highlight_index < (int) portfolio_data->length - 1) {
            highlight_index++;
            if (scroll_on && highlight_index == getmaxy(list_window) + scroll_index) // Scroll down
                scroll_index++;
            // If key down and not highlighting first index
        } else if (ch == KEY_UP && highlight_index > 0) {
            highlight_index--;
            if (scroll_on && highlight_index == scroll_index - 1) // Scroll up
                scroll_index--;
        }
    } while (ch != 'q'); // "q" to quit
    endwin();
}

void portfolio_print_stock(const char* slug) {
    String* pString = portfolio_ncurses_get_plaintext_string(NULL);
    if (pString == NULL)
        return;

    Json* jobj = json_tokener_parse(pString->data);
    size_t i = 0, len = json_object_array_length(jobj);
    while (i < len && !streq(json_object_get_string(json_object_object_get
    (json_object_array_get_idx(jobj, i), "Symbol")), slug))
        i++;

    if (i == len)
        GOTO_CLEAN_MSG("Your portfolio does not contain any of this security.")

    Info* info = info_init();
    strcpy(info->slug, slug);
    api_store_info(info, DATA_LEVEL_CHECK);

    info->amount = json_object_get_double(json_object_object_get(json_object_array_get_idx(jobj, i), "Shares"));
    info->total_spent = json_object_get_double(json_object_object_get(json_object_array_get_idx(jobj, i), "USD_Spent"));
    info_store_portfolio_data(info);
    printf("  AMOUNT SYMBOL    VALUE    SPENT   PROFIT       (%%)      24H       (%%)      7D    "
           "    (%%)      30D      "
           " (%%)\n%8.2lf %6s %8.2lf %8.2lf %8.2lf (%6.2lf%%) %8.2lf (%6.2lf%%) %8.2lf (%6.2lf%%) %8.2lf (%6.2lf%%)\n",
           info->amount, info->symbol, info->current_value, info->total_spent, info->profit_total,
           info->profit_total_percent, info->profit_last_close, info->profit_last_close_percent,
           info->profit_7d, info->profit_7d_percent, info->profit_30d, info->profit_30d_percent);

    info_destroy(&info);
    cleanup:
    json_object_put(jobj);
    string_destroy(&pString);
}

void interface_print(const char* slug) {
    Info* pInfo = info_init();
    strcpy(pInfo->slug, slug);
    api_store_info(pInfo, DATA_LEVEL_ALL);
    if (pInfo->api_provider == EMPTY) {
        info_destroy(&pInfo);
        RET_MSG("Invalid slug.")
    }

    if (pInfo->points == NULL || pInfo->name[0] == '\0') { // If not IEX print to stdout
        info_print(pInfo);
        info_destroy(&pInfo);
        return;
    }

    initscr();
    if (!has_colors())
        GOTO_CLEAN_MSG("Your terminal does not support color.")

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    start_color();
    init_pair(2, COLOR_WHITE, COLOR_BLACK); // Init black background, white foreground
    bkgd(BLACK); // set background/foreground
    curs_set(FALSE);
    WINDOW* header_window = newwin(2, cols - 13, 0, 13);
    WINDOW* graph_window = newwin(GRAPH_HEIGHT, GRAPH_WIDTH, GRAPH_Y, GRAPH_X);
    WINDOW* company_window = newwin(COMPANY_HEIGHT, COMPANY_WIDTH, COMPANY_Y, COMPANY_X);
    WINDOW* news_window = newwin(NEWS_HEIGHT, NEWS_WIDTH, NEWS_Y, NEWS_X);
    WINDOW* peer_window = newwin(PEERS_HEIGHT, PEERS_WIDTH, PEERS_Y, PEERS_X);
    wbkgd(header_window, BLACK);
    wbkgd(graph_window, BLACK);
    wbkgd(company_window, BLACK);
    wbkgd(news_window, BLACK);
    wbkgd(peer_window, BLACK);

    int graph_rows, graph_cols;
    getmaxyx(graph_window, graph_rows, graph_cols);
    graph_cols -= 11; // 10 offset to give space for graph labels + 1 for right side
    graph_rows -= 3; // Make space for zoom indicator
    graph_rows -= graph_rows % ROWS_SPACING; // Round down to multiple of 5
    if (graph_cols < GRAPH_COLS_MIN || graph_rows < GRAPH_ROWS_MIN) // Exits if the terminal is too small
        GOTO_CLEAN_MSG("Terminal not large enough.")

    header_printw(header_window, pInfo); // Print to windows
    info_printw(company_window, pInfo);
    news_printw(news_window, pInfo);
    peers_printw(peer_window, pInfo);

    wrefresh(header_window); // Refresh other windows before graph otherwise they won't print before next getch()
    wrefresh(company_window);
    wrefresh(news_window);
    wrefresh(peer_window);

    graph_printw(graph_window, pInfo, NULL); // No refresh needed since getch()

    cleanup:
    endwin();
    info_destroy(&pInfo);
}

void header_printw(WINDOW* window, const Info* pInfo) {
    if (pInfo->intraday_time != EMPTY) {
        char time_str[DATE_MAX_LENGTH];
        time_t time = pInfo->intraday_time; // divide into second instead of milliseconds
        struct tm* ts = localtime(&time);
        strftime(time_str, DATE_MAX_LENGTH, "%F %T", ts);
        mvwprintw(window, 0, 0, "%s", time_str);
    }
    mvwprintw(window, 0, (int) (15 + strlen(pInfo->name) + strlen(pInfo->symbol)), "24H      7D     ");
    if (pInfo->price_30d != EMPTY)
        wprintw(window, "30D");
    mvwprintw(window, 1, 0, "%s %s %8.2lf %6.2lf%% %6.2lf%% ", pInfo->name, pInfo->symbol,
             pInfo->price, 100 * (pInfo->price / pInfo->price_last_close - 1),
              100 * (pInfo->price / pInfo->price_7d - 1));
    if (pInfo->price_30d != EMPTY)
        wprintw(window, "%6.2lf%%", 100 * (pInfo->price / pInfo->price_30d - 1));
}

void info_print(const Info* pInfo) {
    if (streq(pInfo->name, ""))
        printf("Name: %s\n", pInfo->name);
    if (streq(pInfo->symbol, ""))
        printf("Symbol: %s\n", pInfo->symbol);
    if (pInfo->price != EMPTY)
        printf("Price: $%lf\n", pInfo->price);
    if (pInfo->price_last_close != EMPTY)
        printf("Percent change 24h: %.2lf%%\n", 100 * (pInfo->price / pInfo->price_last_close - 1));
    if (pInfo->price_7d != EMPTY)
        printf("Percent change 7d: %.2lf%%\n", 100 * (pInfo->price / pInfo->price_7d - 1));
    if (pInfo->price_30d != EMPTY)
        printf("Percent change 30d: %.2lf%%\n", 100 * (pInfo->price / pInfo->price_30d - 1));
    if (pInfo->div_yield != EMPTY)
        printf("Dividend yield: %.2lf%%\n", pInfo->div_yield);
    if (pInfo->marketcap != EMPTY)
        printf("Market Cap: $%ld\n", pInfo->marketcap);
    if (pInfo->volume_1d != EMPTY)
        printf("Volume 24h: $%ld\n", pInfo->volume_1d);
}

void info_printw(WINDOW* window, const Info* pInfo) {
    if (pInfo->description[0] != '\0')
        mvwprintw(window, 0, 0, "%s\n\n", pInfo->description);
    else mvwprintw(window, 0, 0, "Description unavailable.\n\n");

    if (pInfo->ceo[0] != '\0')
        wprintw(window, "CEO: %s", pInfo->ceo);
    else wprintw(window, "CEO unavailable.");

    if (pInfo->website[0] != '\0')
        mvwprintw(window, getcury(window), getmaxx(window) / 2, "Website: %s\n", pInfo->website);
    else mvwprintw(window, getcury(window), getmaxx(window) / 2, "Website unavailable.\n");

    if (pInfo->sector[0] != '\0')
        wprintw(window, "Sector: %s", pInfo->sector);
    else wprintw(window, "Sector unavailable.");

    if (pInfo->industry[0] != '\0')
        mvwprintw(window, getcury(window), getmaxx(window) / 2, "Industry: %s\n", pInfo->industry);
    else mvwprintw(window, getcury(window), getmaxx(window) / 2, "Industry unavailable.\n");

    if (pInfo->revenue != EMPTY)
        wprintw(window, "Revenue: %ld", pInfo->revenue);
    else wprintw(window, "Revenue unavailable.");

    if (pInfo->gross_profit != EMPTY)
        mvwprintw(window, getcury(window), getmaxx(window) / 2, "Gross Profit: %ld\n", pInfo->gross_profit);
    else mvwprintw(window, getcury(window), getmaxx(window) / 2, "Gross Profit unavailable.\n");

    if (pInfo->cash != EMPTY)
        wprintw(window, "Cash: %ld", pInfo->cash);
    else wprintw(window, "Cash unavailable.");

    if (pInfo->debt != EMPTY)
        mvwprintw(window, getcury(window), getmaxx(window) / 2, "Debt: %ld\n", pInfo->debt);
    else mvwprintw(window, getcury(window), getmaxx(window) / 2, "Debt unavailable.\n");

    if (pInfo->marketcap != EMPTY)
        wprintw(window, "Market Cap: %ld", pInfo->marketcap);
    else wprintw(window, "Market Cap unavailable.");

    if (pInfo->volume_1d != EMPTY)
        mvwprintw(window, getcury(window), getmaxx(window) / 2, "Volume: %ld\n", pInfo->volume_1d);
    else mvwprintw(window, getcury(window), getmaxx(window) / 2, "Volume unavailable.\n");

    if (pInfo->pe_ratio != EMPTY)
        wprintw(window, "P/E Ratio: %lf", pInfo->pe_ratio);
    else wprintw(window, "P/E Ratio unavailable.");

    if (pInfo->div_yield != EMPTY)
        mvwprintw(window, getcury(window), getmaxx(window) / 2, "Dividend Yield: %lf\n\n", pInfo->div_yield);
    else mvwprintw(window, getcury(window), getmaxx(window) / 2, "Dividend Yield unavailable.\n\n");

    for (int i = 0; i < QUARTERS && pInfo->fiscal_period[i][0] != '\0'; i++)
        mvwprintw(window, getcury(window), 4 + i * getmaxx(window) / QUARTERS, "%s",
                  pInfo->fiscal_period[i]);
    waddch(window, '\n');

    if (pInfo->eps[0] != EMPTY)
        mvwprintw(window, getcury(window), 0, "EPS ");
    for (int i = 0; i < QUARTERS && pInfo->eps[i] != EMPTY; i++)
        mvwprintw(window, getcury(window), 4 + i * getmaxx(window) / QUARTERS, "%.2lf",
                  pInfo->eps[i]);
    waddch(window, '\n');

    if (pInfo->eps_year_ago[0] != EMPTY)
        mvwprintw(window, getcury(window), 0, "1Y  ");
    for (int i = 0; i < QUARTERS && pInfo->eps_year_ago[i] != EMPTY; i++)
        mvwprintw(window, getcury(window), 4 + i * getmaxx(window) / QUARTERS, "%.2lf", pInfo->eps_year_ago[i]);
}

void news_print(const char* slug, int num_articles) {
    if (num_articles > 50 || num_articles < 1)
        RET_MSG("You cannot request more than 50 articles.");

    Info* pInfo = info_init();
    strcpy(pInfo->slug, slug);
    pInfo->num_articles = num_articles;
    api_store_info(pInfo, DATA_LEVEL_NEWS);
    if (pInfo->api_provider == EMPTY) {
        info_destroy(&pInfo);
        RET_MSG("Invalid slug");
    }
    for (int i = 0; i < pInfo->num_articles; i++)
        printf("%s | %s | %s\n%s\n%s | Related: %s\n\n",
               pInfo->articles[i]->headline, pInfo->articles[i]->source, pInfo->articles[i]->date,
               pInfo->articles[i]->summary, pInfo->articles[i]->url, pInfo->articles[i]->related);
    info_destroy(&pInfo);
}

void news_printw(WINDOW* window, const Info* pInfo) {
    if (pInfo->num_articles == EMPTY)
        wprintw(window, "News unavailable.");
    for (int i = 0; i < pInfo->num_articles;  i++)
        wprintw(window, "%s | %s | %s\n%s\n%s | Related: %s\n\n",
               pInfo->articles[i]->headline, pInfo->articles[i]->source, pInfo->articles[i]->date,
               pInfo->articles[i]->summary, pInfo->articles[i]->url, pInfo->articles[i]->related);
}

void peers_printw(WINDOW* window, const Info* pInfo) {
    if (pInfo->peers == NULL) {
        wprintw(window, "Peers unavailable.");
        return;
    }

    wprintw(window, "Peers:\n\nSYMBOL    PRICE    24H%%     7D%%    30D%%");
    Info* idx;
    for (size_t i = 0; i < pInfo->peers->length; i++) {
        idx = pInfo->peers->array[i];
        mvwprintw(window, (int) i + 3, 0, "%6s %8.2lf %6.2lf%% %6.2lf%% %6.2lf%%", idx->symbol, idx->price,
                  100 * (idx->price / idx->price_last_close - 1), 100 * (idx->price / idx->price_7d - 1),
                  100 * (idx->price / idx->price_30d - 1));
    }
}

void graph_print(const char* slug, const char* slug2) {
    Info* pInfo = info_init(), * pInfo2 = NULL;
    strcpy(pInfo->slug, slug);
    api_store_info(pInfo, DATA_LEVEL_GRAPH);
    if (pInfo->api_provider == EMPTY || pInfo->points == NULL) {
        info_destroy(&pInfo);
        RET_MSG("Invalid slug")
    }

    if (slug2 != NULL) {
        pInfo2 = info_init();
        strcpy(pInfo2->slug, slug2);
        api_store_info(pInfo2, DATA_LEVEL_GRAPH);
        if (pInfo2->api_provider ==  EMPTY || pInfo2->points == NULL) {
            info_destroy(&pInfo);
            info_destroy(&pInfo2);
            RET_MSG("Invalid slug")
        }
    }
    initscr();
    if (!has_colors()) { // compare command will use two colors to differentiate, so color must
        endwin();        // be supported. Must endwin() before puts()
        RET_MSG("Your terminal does not support color.");
    }

    graph_printw(stdscr, pInfo, pInfo2);
    info_destroy(&pInfo);
    info_destroy(&pInfo2);
    endwin();
}

void graph_printw(WINDOW* window, Info* pInfo, Info* pInfo2) {
    noecho(); // Don't echo keystrokes
    keypad(window, TRUE); // Enables extra keystrokes
    curs_set(0); // Hides cursor
    start_color(); // Enable colors for comparison
    init_pair(1, COLOR_RED, COLOR_BLACK); // Init color red
    init_pair(2, COLOR_WHITE, COLOR_BLACK); // Init black background, white foreground
    wbkgd(window, BLACK); // set background/foreground

    time_t now = time(NULL);
    struct tm today_date = *localtime(&now), start_date = today_date, furthest_back_date = today_date, end;
    start_date.tm_year -= 5, furthest_back_date.tm_year -= 5; // Go back five years
    double seconds = difftime(mktime(&today_date), mktime(&furthest_back_date));

    // Calculate total number of trading days between today and five years ago
    int trading_days = (int) ((1.0 / DAYS_TO_BUSINESS_DAYS_RATIO) * seconds / 86400.0);

    // If younger than 5 years, realloc with num of trading days and fill with EMPTY
    if (trading_days - pInfo->num_points > 0)
        info_chart_fill_empty(pInfo, trading_days);

    if (pInfo2 != NULL && trading_days - pInfo2->num_points > 0)
        info_chart_fill_empty(pInfo2, trading_days);

    int ch, zoom = ZOOM_5y;
    graph_draw(window, pInfo, pInfo2, &start_date, zoom); // Initial graph of 5 year history

    while ((ch = wgetch(window)) != 'q') { // Main input loop -- end if keypress 'q'
        if ((ch == KEY_UP && zoom != ZOOM_1m) || (ch == KEY_DOWN && zoom != ZOOM_5y) ||
            (zoom != ZOOM_5y && (ch == KEY_LEFT || ch == KEY_RIGHT))) { // UP / DOWN / LEFT / RIGHT
            if (ch == KEY_UP) {
                zoom++;
                start_date.tm_mon += zoom_change_x_months[zoom];
            } else if (ch == KEY_RIGHT || ch == KEY_DOWN) {
                if (ch == KEY_DOWN)
                    zoom--;
                if (zoom == ZOOM_1m)
                    start_date.tm_mon += 1; // Moves only one month if zoom is 1 month
                else start_date.tm_mon += zoom_change_x_months[zoom];
            } else { // LEFT
                if (zoom == ZOOM_1m)
                    start_date.tm_mon -= 1; // Moves only one month if zoom is 1 month
                else start_date.tm_mon -= zoom_change_x_months[zoom];
                if (difftime(mktime(&start_date), mktime(&furthest_back_date)) < 0)
                    start_date = furthest_back_date; // Can't go back past furthest_date
            }
            end = start_date; // If trying to go past current date, set start to (zoom level - current date)
            end.tm_mon += zoom_months[zoom];
            if (difftime(mktime(&end), mktime(&today_date)) > 0) {
                start_date = today_date;
                start_date.tm_mon -= zoom_months[zoom];
            }
            graph_draw(window, pInfo, pInfo2, &start_date, zoom);
        }
    }
}

void graph_draw(WINDOW* window, Info* pInfo, Info* pInfo2, struct tm* start_time, int zoom) {
    wmove(window, 0, 0); // Instead of clear()ing, move to the top left corner and re-print
    int cols, rows;
    getmaxyx(window, rows, cols);
    cols -= 11; // 10 offset to give space for graph labels + 1 for right side
    rows -= 3; // Make space for zoom indicator
    rows -= rows % ROWS_SPACING; // Round down to multiple of 5
    if (cols < GRAPH_COLS_MIN || rows < GRAPH_ROWS_MIN) // Exits if the terminal is too small
        RET_MSG("Terminal not large enough.")

    time_t now = time(NULL);
    struct tm end_date = *start_time, * five_y = localtime(&now);
    end_date.tm_mon += zoom_months[zoom], five_y->tm_year -= 5;

    double seconds = difftime(mktime(&end_date), mktime(start_time));
    int trading_days = (int) ((1.0 / DAYS_TO_BUSINESS_DAYS_RATIO) * seconds / 86400.0); // Total trading days to print

    seconds = difftime(mktime(start_time), mktime(five_y));
    int starting_index = (int) ((1.0 / DAYS_TO_BUSINESS_DAYS_RATIO) * seconds / 86400.0);

    double max = pInfo->points[starting_index], min = pInfo->points[starting_index];
    int k = 0;
    while (max == EMPTY) // If initial max is EMPTY, get first non-EMPTY value
        max = pInfo->points[++k];
    if (k > 0) // Do the same thing with min
        min = max;
    for (int i = starting_index + 1; i < trading_days + starting_index; i++) {
        if (pInfo->points[i] != EMPTY) { // Ignore EMPTY values
            if (pInfo->points[i] > max) // Find max and min values for graph upper/lower bounds
                max = pInfo->points[i];
            if (pInfo->points[i] < min)
                min = pInfo->points[i];
        }
    }
    double line_diff = (max - min) / rows, day_close; // Each line includes data point up to line_diff below

    double max2 = 0, min2 = 0, line_diff2 = 0, day_close2 = 0;
    if (pInfo2 != NULL) {
        max2 = pInfo2->points[starting_index], min2 = pInfo2->points[starting_index];
        k = 0;
        while (max2 == EMPTY) // If initial max is EMPTY, get first non-EMPTY value
            max2 = pInfo2->points[++k];
        if (k > 0) // Do the same thing with min
            min2 = max2;
        for (int i = starting_index + 1; i < trading_days + starting_index; i++) {
            if (pInfo2->points[i] != EMPTY) { // Ignore EMPTY values
                if (pInfo2->points[i] > max2) // Find max and min values for graph upper/lower bounds
                    max2 = pInfo2->points[i];
                if (pInfo2->points[i] < min2)
                    min2 = pInfo2->points[i];
            }
        }
        line_diff2 = (max2 - min2) / rows; // Each line includes data point up to line_diff below
    }

    for (int i = rows; i >= 0; i--) {
        if (i % ROWS_SPACING == 0) // Print y-axis price labels with width 10
            wprintw(window, "%9.2lf ", (max - ((rows - i) * line_diff)));
        else if (pInfo2 != NULL && (i - 1) % ROWS_SPACING == 0) { // Print comparison price label above
            wattron(window, RED);
            wprintw(window, "%9.2lf ", (max2 - ((rows - i) * line_diff2)));
            wattroff(window, RED);
        } else wprintw(window, "          "); // Indent width 10 otherwise

        for (int j = 0; j < cols; j++) {
            day_close = pInfo->points[starting_index + (int) ((double) j * trading_days / cols)]; // Get close prices
            if (pInfo2 != NULL)
                day_close2 = pInfo2->points[starting_index + (int) ((double) j * trading_days / cols)];

            if (day_close <= (max - ((rows - i) * line_diff)) && day_close > (min + ((i - 1) * line_diff)))
                waddch(window, ACS_DIAMOND); // Print diamond if close price is within line_diff
            else if (pInfo2 != NULL && day_close2 <= (max2 - ((rows - i) * line_diff2)) &&
                     day_close2 > (min2 + ((i - 1) * line_diff2))) {
                wattron(window, RED);
                waddch(window, ACS_DIAMOND); // Print RED diamond if close price is within line_diff
                wattroff(window, RED);
            } else if (i % ROWS_SPACING == 0 && j % COLS_SPACING == 0) // Cross on corners
                waddch(window, ACS_PLUS);
            else if (i % ROWS_SPACING == 0) // Horizontal line every ROWS_SPACING lines
                waddch(window, ACS_HLINE);
            else if (j % COLS_SPACING == 0) // Vertical line every COLS_SPACING lines
                waddch(window, ACS_VLINE);
            else // Otherwise prints a space
                waddch(window, ' ');
        }
        waddch(window, '\n'); // Newline on line end
    }

    wprintw(window, "     "); // Indent to center date labels
    char time_string[16];
    double days_per_col_spacing = (DAYS_TO_BUSINESS_DAYS_RATIO * trading_days) / (cols / COLS_SPACING);
    struct tm copy = *start_time;
    for (int i = 0; i < cols; i++) {
        if (i % (2 * COLS_SPACING) == 0 && cols - i > 5) { // Print x-axis date labels every two
            if (i != 0)
                copy.tm_sec += days_per_col_spacing * 2.0 * 86400.0;
            mktime(&copy);
            strftime(time_string, 16, "%m/%d/%Y", &copy);
            wprintw(window, "%s              ", time_string); // Width 2 * COLS_SPACING
        }
    }

    waddch(window, '\n');
    waddch(window, '\n');
    wprintw(window, "%s", pInfo->symbol); // Empty line as spacing, then print key containing the symbol(s) and diamond
    // with color
    waddch(window, ACS_DIAMOND);
    if (pInfo2 != NULL) {
        wattron(window, RED);
        wprintw(window, " %s", pInfo2->symbol);
        waddch(window, ACS_DIAMOND);
        wattroff(window, RED);
    }
    waddch(window, ' ');
    size_t offset = (cols / 2) - (11 + strlen(pInfo->symbol)); // Center zoom level
    if (pInfo2 != NULL)
        offset -= strlen(pInfo2->symbol) + 2;
    for (unsigned int i = 0; i < offset; i++)
        waddch(window, ' '); // Center text
    const char* str[9] = {"5y", "4y", "3y", "2y", "1y", "9m", "6m", "3m", "1m"}; // Zoom level
    for (int i = 0; i < 9; i++) {
        if (zoom == i)
            wattron(window, A_STANDOUT); // Highlight current zoom level
        waddstr(window, str[i]);
        wattroff(window, A_STANDOUT);
        waddch(window, ' ');
    }
}