#include "api.h"

char* ref_cache_file_path = NULL;

Ref_Data* ref_cache = NULL;

char* crypto_cache_file_path = NULL;

Ref_Data* crypto_cache = NULL;

char* keys_file_path = NULL;

Key_Ring* api_keys = NULL;

char api_abbreviations[API_PROVIDER_MAX][SYMBOL_MAX_LENGTH] = {"IEX", "AV", "CMC"};

char api_names[API_PROVIDER_MAX][NAME_MAX_LENGTH] = {"Investors Exchange (IEX)", "Alpha Vantage",
                                                     "CoinMarketCap"};

char api_websites[API_PROVIDER_MAX][URL_MAX_LENGTH] = {
        "URL TBD", "https://www.alphavantage.co/support/#api-key",
        "https://pro.coinmarketcap.com/signup"};

void keys_file_path_init(void) {
    char* home = getenv("HOME");
    char* path = malloc(strlen(home) + 32);
    pointer_alloc_check(path);
    sprintf(path, "%s/.tick_api_keys.json", home);
    keys_file_path = path; // $HOME/.tick_api_keys.json
}

void ref_cache_file_path_init(void) {
    char* home = getenv("HOME");
    char* path = malloc(strlen(home) + 32);
    pointer_alloc_check(path);
    sprintf(path, "%s/.tick_ref_cache.json", home);
    ref_cache_file_path = path; // $HOME/.tick_ref_cache.json
}

void crypto_cache_file_path_init(void) {
    char* home = getenv("HOME");
    char* path = malloc(strlen(home) + 32);
    pointer_alloc_check(path);
    sprintf(path, "%s/.tick_crypto_cache.json", home);
    crypto_cache_file_path = path; // $HOME/.tick_crypto_cache.json
}

Key_Ring* key_ring_init(void) {
    Key_Ring* pKeys = malloc(sizeof(Key_Ring));
    pointer_alloc_check(pKeys);
    for (int i = 0; i < API_PROVIDER_MAX; i++) {
        pKeys->providers[i] = i;
        pKeys->keys[i][0] = '\0';
    }
    return pKeys;
}

Ref_Data* ref_data_init_length(size_t length) {
    Ref_Data* pRef_Data = malloc(sizeof(Ref_Data));
    pointer_alloc_check(pRef_Data);
    pRef_Data->slugs = malloc(length * sizeof(char*));
    pRef_Data->names = malloc(length * sizeof(char*));
    pointer_alloc_check(pRef_Data->slugs);
    pointer_alloc_check(pRef_Data->names);
    for (size_t i = 0; i < length; i++) {
        pRef_Data->slugs[i] = calloc(SLUG_MAX_LENGTH, sizeof(char));
        pRef_Data->names[i] = calloc(NAME_MAX_LENGTH, sizeof(char));
        pointer_alloc_check(pRef_Data->slugs[i]);
        pointer_alloc_check(pRef_Data->names[i]);
    }
    pRef_Data->length = length;
    pRef_Data->time_loaded = EMPTY;
    return pRef_Data;
}

News* news_init(void) {
    News* pNews = malloc(sizeof(News));
    pointer_alloc_check(pNews);
    *pNews = (News) {
        .headline[0] = '\0', .source[0] = '\0', .date[0] = '\0', .summary[0] = '\0', .url[0] = '\0', .related[0] = '\0'
    };
    return pNews;
}

Info* info_init(void) {
    Info* pInfo = malloc(sizeof(Info));
    pointer_alloc_check(pInfo);
    *pInfo = (Info) { // Null terminate every string and set every value to EMPTY
            .api_provider = EMPTY, .slug[0] = '\0', .symbol[0] = '\0', .name[0] = '\0',
            .industry[0] ='\0', .website[0] = '\0', .description[0] = '\0', .ceo[0] = '\0',
            .issue_type[0] = '\0', .sector[0] = '\0', .intraday_time = EMPTY, .price = EMPTY,
            .marketcap = EMPTY, .volume_1d = EMPTY, .pe_ratio = EMPTY, .div_yield = EMPTY,
            .revenue = EMPTY, .gross_profit = EMPTY, .cash = EMPTY, .debt = EMPTY,
            .eps = {EMPTY, EMPTY, EMPTY, EMPTY}, .fiscal_period[0][0] = '\0',
            .fiscal_period[1][0] = '\0', .fiscal_period[2][0] = '\0', .fiscal_period[3][0] = '\0',
            .eps_year_ago = {EMPTY, EMPTY, EMPTY, EMPTY}, .price_last_close = EMPTY,
            .price_7d = EMPTY, .price_30d = EMPTY, .points = NULL, .num_points = EMPTY,
            .articles = NULL, .num_articles = EMPTY, .peers = NULL, .amount = EMPTY,
            .total_spent = EMPTY, .current_value = 0, .famount[0] = '\0', .ftotal_spent[0] = '\0',
            .fcurrent_value[0] = '\0', .profit_total = EMPTY, .profit_total_percent = EMPTY,
            .profit_last_close = EMPTY, .profit_last_close_percent = EMPTY, .profit_7d = EMPTY,
            .profit_7d_percent = EMPTY, .profit_30d = EMPTY, .profit_30d_percent = EMPTY,
            .fprofit_total[0] = '\0', .fprofit_total_percent[0] = '\0',
            .fprofit_last_close[0] = '\0', .fprofit_last_close_percent[0] = '\0',
            .fprofit_7d[0] = '\0', .fprofit_7d_percent[0] = '\0', .fprofit_30d[0] = '\0',
            .fprofit_30d_percent[0] = '\0'
    };
    return pInfo;
}

Info_Array* info_array_init(void) {
    Info_Array* pInfo_Array = malloc(sizeof(Info_Array));
    pointer_alloc_check(pInfo_Array);
    *pInfo_Array = (Info_Array) {
            .array = NULL, .length = 0, .totals = NULL
    };
    return pInfo_Array;
}

Info_Array* info_array_init_length(size_t length) {
    if (length == 0)
        return info_array_init();

    Info_Array* pInfo_Array = info_array_init();
    pInfo_Array->length = length;
    pInfo_Array->array = malloc(sizeof(Info*) * length);
    pointer_alloc_check(pInfo_Array->array);
    for (size_t i = 0; i < length; i++)
        pInfo_Array->array[i] = info_init();

    pInfo_Array->totals = info_init();
    strcpy(pInfo_Array->totals->slug, "TOTALS");
    strcpy(pInfo_Array->totals->symbol, "TOTALS");
    strcpy(pInfo_Array->totals->name, "TOTALS");
    return pInfo_Array;
}

void info_array_append(Info_Array* pInfo_Array, const char* slug) {
    pInfo_Array->array = realloc(pInfo_Array->array, sizeof(char*) * (pInfo_Array->length + 1));
    pInfo_Array->length++;
    pInfo_Array->array[pInfo_Array->length - 1] = info_init();
    strcpy(pInfo_Array->array[pInfo_Array->length - 1]->slug, slug);
}

size_t string_writefunc(void* ptr, size_t size, size_t nmemb, String* pString) {
    size_t new_len = pString->len + size * nmemb;
    pString->data = realloc(pString->data, new_len + 1);
    pointer_alloc_check(pString->data);
    memcpy(pString->data + pString->len, ptr, size * nmemb);
    pString->data[new_len] = '\0';
    pString->len = new_len;
    return size * nmemb;
}

String* api_curl_url(const char* url) {
    CURL* curl = curl_easy_init();
    CURLcode res;
    if (!curl) // Error creating curl object
        RETNULL_MSG("Error initializing curl.")

    String* pString = string_init();
    curl_easy_setopt(curl, CURLOPT_URL, url); // Set URL
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Needed for HTTPS
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, string_writefunc); // Specify writefunc for return data
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pString->data); // Specify object for return data
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        string_destroy(&pString); // Error receiving data
        RETNULL_MSG("Error curling data.")
    }

    return pString;
}

void api_iex_store_info_array(Info_Array* pInfo_Array, Data_Level data_level) {
    char** slug_array = malloc(pInfo_Array->length * sizeof(char*));
    pointer_alloc_check(slug_array);
    for (size_t i = 0; i < pInfo_Array->length; i++) {
        slug_array[i] = malloc(SYMBOL_MAX_LENGTH);
        pointer_alloc_check(slug_array[i]);
        strcpy(slug_array[i], pInfo_Array->array[i]->slug);
    }

    String* pString = api_iex_get_data_string(slug_array, pInfo_Array->length, data_level);
    if (pString == NULL) { // No internet connection
        for (size_t i = 0; i < pInfo_Array->length; i++)
            free(slug_array[i]);

        free(slug_array);
        return;
    }

    Json* jobj = json_tokener_parse(pString->data);
    info_array_store_endpoints_json(pInfo_Array, jobj);

    for (size_t i = 0; i < pInfo_Array->length; i++)
        free(slug_array[i]);

    free(slug_array);
    json_object_put(jobj);
    string_destroy(&pString);
}

void api_iex_store_info(Info* pInfo, Data_Level data_level) {
    char* slug_array = malloc(SYMBOL_MAX_LENGTH);
    pointer_alloc_check(slug_array);
    strcpy(slug_array, pInfo->slug);
    String* pString = api_iex_get_data_string(&slug_array, 1, data_level);
    Json* jobj = json_tokener_parse(pString->data);
    Json* jslug = json_object_object_get(jobj, pInfo->slug);
    if (jslug != NULL)
        info_store_endpoints_json(pInfo, jslug);

    free(slug_array);
    json_object_put(jobj);
    string_destroy(&pString);
}

String* api_iex_get_data_string(char** slug_array, size_t len,
                                Data_Level data_level) {
    char endpoints[128];
    if (data_level == DATA_LEVEL_ALL)
        strcpy(endpoints, "quote,chart,company,stats,peers,news,earnings&range=5y");
    else if (data_level == DATA_LEVEL_CHECK)
        strcpy(endpoints, "quote,chart");
    else if (data_level == DATA_LEVEL_GRAPH)
        strcpy(endpoints, "quote,chart&range=5y");
    else if (data_level == DATA_LEVEL_MISC)
        strcpy(endpoints, "chart,company,stats,peers,news,earnings&range=5y");
    else strcpy(endpoints, "news");

    size_t num_partitions = len / 100 + (len % 100 > 0), idx;
    char iex_api_string[num_partitions][URL_MAX_LENGTH * 2];
    char slug_list_string[num_partitions][INFO_MAX_LENGTH];
    memset(slug_list_string, '\0', num_partitions * INFO_MAX_LENGTH);
    String* string_array[num_partitions];
    pthread_t threads[num_partitions];
    for (size_t i = 0; i < num_partitions; i++) {
        for (size_t j = 0; j < 100; j++) {
            idx = i * 100 + j;
            if (idx == len)
                break;

            if (!streq(slug_array[idx], "USD$"))
                sprintf(&slug_list_string[i][strlen(slug_list_string[i])], "%s,",
                        slug_array[idx]);
        }
        slug_list_string[i][strlen(slug_list_string[i]) - 1] = '\0'; // Remove last comma
        sprintf(iex_api_string[i],
                "https://api.iextrading.com/1.0/stock/market/batch?symbols=%s&types=%s",
                slug_list_string[i], endpoints);
        if (pthread_create(&threads[i], NULL, (void* (*)(void*)) api_curl_url,
                           (void*) iex_api_string[i]))
            EXIT_MSG("Error creating thread!");
    }

    for (size_t i = 0; i < num_partitions; i++)
        if (!streq(slug_array[i], "USD$") &&
            pthread_join(threads[i], (void**) &string_array[i]))
            EXIT_MSG("Error joining thread!");

    if (num_partitions > 1) {
        size_t total_length = 0;
        for (size_t i = 0; i < num_partitions; i++)
            total_length += string_array[i]->len;

        string_array[0]->len = total_length;
        string_array[0]->data = realloc(string_array[0]->data, string_array[0]->len + 1);
        pointer_alloc_check(string_array[0]->data);
        for (size_t i = 1; i < num_partitions; i++) {
            sprintf(&string_array[0]->data[strlen(string_array[0]->data) - 1], ",%s",
                    &string_array[i]->data[1]);
            string_destroy(&string_array[i]);
        }
    }

    return string_array[0];
}

void* api_alphavantage_store_info(void* vpInfo) {
    Info* pInfo = vpInfo;
    if (api_keys->keys[API_PROVIDER_ALPHAVANTAGE][0] == '\0' || pInfo->slug[0] == '\0')
        return NULL;

    char alphavantage_api_string[URL_MAX_LENGTH];
    sprintf(alphavantage_api_string, "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY"
                                     "&symbol=%s&apikey=%s&outputsize=full&datatype=csv",
                                     pInfo->slug, api_keys->keys[API_PROVIDER_ALPHAVANTAGE]);
    String* pString = api_curl_url(alphavantage_api_string);
    if (pString == NULL)
        return NULL;

    if (pString->data[0] == '{') { // Invalid slug/error
        string_destroy(&pString);
        return NULL;
    }

    pInfo->api_provider = API_PROVIDER_ALPHAVANTAGE;
    strcpy(pInfo->symbol, pInfo->slug);

    pInfo->num_points = (int) string_get_num_lines(pString) - 1;
    size_t idx = 0;
    if (pInfo->num_points > 1260) // 5 years
        pInfo->num_points = 1260;

    pInfo->points = calloc((size_t) pInfo->num_points, sizeof(double));
    pointer_alloc_check(pInfo->points);

    csv_goto_next_line(pString, &idx); // skip columns line
    for (int i = pInfo->num_points - 1; i >= 0; i--) {
        for (size_t j = 0; j < 4; j++)
            csv_goto_next_value(pString, &idx);

        pInfo->points[i] = csv_read_next_double(pString, &idx);
        if (pInfo->points[i] == 0 && i < pInfo->num_points - 1) // API Error
            pInfo->points[i] = pInfo->points[i + 1];
        csv_goto_next_line(pString, &idx);
    }

    pInfo->price = pInfo->points[pInfo->num_points - 1];
    pInfo->price_last_close = pInfo->points[pInfo->num_points - 2];
    pInfo->price_7d = pInfo->points[pInfo->num_points - 6];
    pInfo->price_30d = pInfo->points[pInfo->num_points - 22];
    string_destroy(&pString);
    return vpInfo;
}

void* api_coinmarketcap_store_info(void* vpInfo) {
    Info* pInfo = vpInfo;
    char coinmarketcap_api_string[URL_MAX_LENGTH];
    sprintf(coinmarketcap_api_string, "https://api.coinmarketcap.com/v1/ticker/%s", pInfo->slug);
    String* pString = api_curl_url(coinmarketcap_api_string);
    if (pString == NULL)
        return NULL;

    if (pString->data[0] == '{') { // Invalid slug
        string_destroy(&pString);
        return NULL;
    }

    pInfo->api_provider = API_PROVIDER_COINMARKETCAP;

    Json* jobj = json_tokener_parse(pString->data);
    Json* data = json_object_array_get_idx(jobj, 0);
    strncpy(pInfo->name, json_object_get_string(json_object_object_get(data, "name")),
            NAME_MAX_LENGTH - 1);
    pInfo->name[NAME_MAX_LENGTH - 1] = '\0';
    strcpy(pInfo->symbol, json_object_get_string(json_object_object_get(data, "symbol")));
    pInfo->price = strtod(json_object_get_string(json_object_object_get(data, "price_usd")), NULL);
    pInfo->price_last_close = pInfo->price /
            (strtod(json_object_get_string(json_object_object_get(data, "percent_change_24h")), NULL) / 100 + 1);
    pInfo->price_7d = pInfo->price /
            (strtod(json_object_get_string(json_object_object_get(data, "percent_change_7d")), NULL) / 100 + 1);
    pInfo->price_30d = pInfo->price_7d;
    pInfo->marketcap = strtol(json_object_get_string(json_object_object_get(data, "market_cap_usd")), NULL, 10);
    pInfo->volume_1d = strtol(json_object_get_string(json_object_object_get(data, "24h_volume_usd")), NULL, 10);
    pInfo->intraday_time = strtol(json_object_get_string(json_object_object_get(data, "last_updated")), NULL, 10);
    json_object_put(jobj);
    string_destroy(&pString);
    return vpInfo;
}

void api_store_info_array(Info_Array* pInfo_Array, Data_Level data_level) {
    api_iex_store_info_array(pInfo_Array, data_level);

    // All IEX securities are accounted for
    Info* pInfo;
    pthread_t threads[pInfo_Array->length];
    int open_threads[pInfo_Array->length];
    memset(open_threads, 0, pInfo_Array->length * sizeof(int));
    for (size_t i = 0; i < pInfo_Array->length; i++) {
        pInfo = pInfo_Array->array[i];
        if (pInfo->api_provider == EMPTY && !streq(pInfo->slug, "USD$")) {
            open_threads[i] = 1;
            if (pthread_create(&threads[i], NULL, api_coinmarketcap_store_info, pInfo)) // Crypto
                EXIT_MSG("Error creating thread!");
        }
    }

    // All IEX and CMC are accounted for
    for (size_t i = 0; i < pInfo_Array->length; i++) {
        pInfo = pInfo_Array->array[i];
        if (open_threads[i]) { // Join all open threads
            if (pthread_join(threads[i], NULL))
                EXIT_MSG("Error joining thread!");

            open_threads[i] = 0;
        }

        if (pInfo->api_provider == EMPTY && !streq(pInfo->slug, "USD$")) {
            open_threads[i] = 1;
            if (pthread_create(&threads[i], NULL, api_alphavantage_store_info, pInfo))
                EXIT_MSG("Error creating thread!");
        }
    }

    // All accounted for
    for (size_t i = 0; i < pInfo_Array->length; i++) {
        if (open_threads[i] && pthread_join(threads[i], NULL))
            EXIT_MSG("Error joining thread!");

        info_store_portfolio_data(pInfo_Array->array[i]);
    }
    info_array_store_totals(pInfo_Array);
}

void api_store_info(Info* pInfo, Data_Level data_level) {
    api_iex_store_info(pInfo, data_level);
    if (pInfo->api_provider == EMPTY && api_coinmarketcap_store_info(pInfo) == NULL &&
        api_alphavantage_store_info(pInfo) == NULL)
        return;

    info_store_portfolio_data(pInfo);
}

void info_store_portfolio_data(Info* pInfo) {
    if (!streq(pInfo->slug, "USD$")) {
        if (pInfo->amount != EMPTY) {
            pInfo->current_value = pInfo->amount * pInfo->price;
            pInfo->profit_total = pInfo->current_value - pInfo->total_spent;
            pInfo->profit_total_percent = 100 * (pInfo->current_value / pInfo->total_spent - 1);
            pInfo->profit_last_close = pInfo->amount * (pInfo->price - pInfo->price_last_close);
            pInfo->profit_7d = pInfo->amount * (pInfo->price - pInfo->price_7d);
            pInfo->profit_30d = pInfo->amount * (pInfo->price - pInfo->price_30d);
        }
        pInfo->profit_last_close_percent = 100 * (pInfo->price / pInfo->price_last_close - 1);
        pInfo->profit_7d_percent = 100 * (pInfo->price / pInfo->price_7d - 1);
        pInfo->profit_30d_percent = 100 * (pInfo->price / pInfo->price_30d - 1);
    } else {
        if (pInfo->amount != EMPTY) {
            pInfo->current_value = pInfo->amount;
            pInfo->profit_total = pInfo->current_value - pInfo->total_spent;
            pInfo->profit_total_percent = 100 * pInfo->profit_total / pInfo->total_spent;
        }
        pInfo->profit_last_close = 0;
        pInfo->profit_last_close_percent = 0;
        pInfo->profit_7d = 0;
        pInfo->profit_7d_percent = 0;
        pInfo->profit_30d = 0;
        pInfo->profit_30d_percent = 0;
    }
}

void info_array_store_totals(Info_Array* pInfo_Array) {
    pInfo_Array->totals->total_spent = 0;
    pInfo_Array->totals->current_value = 0;
    pInfo_Array->totals->profit_total = 0;
    pInfo_Array->totals->profit_last_close = 0;
    pInfo_Array->totals->profit_7d = 0;
    pInfo_Array->totals->profit_30d = 0;
    for (size_t i = 0; i < pInfo_Array->length; i++) {
        pInfo_Array->totals->total_spent += pInfo_Array->array[i]->total_spent;
        pInfo_Array->totals->current_value += pInfo_Array->array[i]->current_value;
        pInfo_Array->totals->profit_total += pInfo_Array->array[i]->profit_total;
        pInfo_Array->totals->profit_last_close += pInfo_Array->array[i]->profit_last_close;
        pInfo_Array->totals->profit_7d += pInfo_Array->array[i]->profit_7d;
        pInfo_Array->totals->profit_30d += pInfo_Array->array[i]->profit_30d;
    }
    pInfo_Array->totals->profit_total_percent = (100 * (pInfo_Array->totals->current_value -
                                                        pInfo_Array->totals->total_spent)) / pInfo_Array->totals->total_spent;
    pInfo_Array->totals->profit_last_close_percent = 100 *
                                                     pInfo_Array->totals->profit_last_close / pInfo_Array->totals->total_spent;
    pInfo_Array->totals->profit_7d_percent = 100 *  pInfo_Array->totals->profit_7d /
                                             pInfo_Array->totals->total_spent;
    pInfo_Array->totals->profit_30d_percent = 100 *  pInfo_Array->totals->profit_30d /
                                              pInfo_Array->totals->total_spent;
}


void keys_init(void) {
    api_keys = key_ring_read();
    for (int i = 0; i < API_PROVIDER_MAX; i++) {
        if (i != API_PROVIDER_IEX && // UNCOMMENT FOR IEXV2
            api_keys->keys[i][0] == '\0')
            printf("API key for %s not set. Please obtain a free API key from \"%s\" and run \"$ "
                   "tick key %s [key]\"\n", api_names[i], api_websites[i], api_abbreviations[i]);
    }
}

void key_ring_write(const Key_Ring* keys) {
    Json* array = json_object_new_array();
    for (Api_Provider i = 0; i < API_PROVIDER_MAX; i++) {
        Json* jobj = json_object_new_object();
        json_object_object_add(jobj, "provider", json_object_new_string(api_abbreviations[i]));
        json_object_object_add(jobj, "key", json_object_new_string(keys->keys[i]));
        json_object_array_add(array, jobj);
    }

    String* pString = string_init_c_string(json_object_to_json_string(array));
    string_write_file(pString, keys_file_path);

    json_object_put(array);
    string_destroy(&pString);
}

Key_Ring* key_ring_read(void) {
    Key_Ring* pKeys = key_ring_init();
    String* file_string = file_get_string(keys_file_path);
    if (file_string == NULL)
        return pKeys;

    Json* jobj = json_tokener_parse(file_string->data), * idx;
    if (jobj == NULL)
        return pKeys;

    for (size_t i = 0; i < json_object_array_length(jobj); i++) {
        idx = json_object_array_get_idx(jobj, i);
        for (Api_Provider j = 0; j < API_PROVIDER_MAX; j++)
            if (streq(json_object_get_string(json_object_object_get(idx, "provider")),
                      api_abbreviations[j]))
                strcpy(pKeys->keys[j], json_object_get_string(json_object_object_get(idx, "key")));
    }

    json_object_put(jobj);
    string_destroy(&file_string);
    return pKeys;
}

void key_ring_add_key(Key_Ring* keys, Api_Provider provider, const char new_key[KEY_MAX_LENGTH]) {
    strcpy(keys->keys[provider], new_key);
    key_ring_write(keys);
}

void api_ref_cache_init(void) {
    Ref_Data* pRef_Data = ref_data_read_cache();
    if (pRef_Data != NULL && difftime(time(NULL), pRef_Data->time_loaded) < 60 * 60 * 24 * 7) {
        ref_cache = pRef_Data;
        return;
    }

    api_ref_data_write_cache();
    ref_cache = ref_data_read_cache();
}

void api_ref_data_write_cache(void) {
    String* pString = api_curl_url("https://api.iextrading.com/1.0/ref-data/symbols");
    if (pString == NULL)
        return;

    string_write_file(pString, ref_cache_file_path);
    string_destroy(&pString);
}

Ref_Data* ref_data_read_cache(void) {
    String* pString = file_get_string(ref_cache_file_path);
    if (pString == NULL)
        return NULL;

    Json* jobj = json_tokener_parse(pString->data);
    if (jobj == NULL) // Return NULL if cache file is not JSON formatted
        return NULL;

    Ref_Data* pRef_Data = ref_data_init_length(json_object_array_length(jobj));
    ref_data_store_json_iex(pRef_Data, jobj);

    json_object_put(jobj);
    string_destroy(&pString);
    return pRef_Data;
}

void api_crypto_cache_init(void) {
    Ref_Data* pRef_Data = crypto_cache_read();
    if (pRef_Data != NULL && difftime(time(NULL), pRef_Data->time_loaded) < 60 * 60 * 24 * 7) {
        crypto_cache = pRef_Data;
        return;
    }

    api_crypto_cache_write();
    crypto_cache = crypto_cache_read();
}

void api_crypto_cache_write(void) {
    char url[URL_MAX_LENGTH];
    sprintf(url, "https://pro-api.coinmarketcap.com/v1/cryptocurrency/listings/latest?sort=name"
                 "&limit=5000&CMC_PRO_API_KEY=%s", api_keys->keys[API_PROVIDER_COINMARKETCAP]);
    String* pString = api_curl_url(url);
    puts("Curled cmc");
    if (pString == NULL)
        return;

    Json* jobj = json_tokener_parse(pString->data);
    Json* data = json_object_object_get(jobj, "data"), * idx;
    for (size_t i = 0; i < json_object_array_length(data); i++) {
        idx = json_object_array_get_idx(data, i);
        json_object_object_del(idx, "quote");
    }

    const char* str = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
    String* smaller = string_init_c_string(str);

    string_write_file(smaller, crypto_cache_file_path);
    json_object_put(jobj);
    string_destroy(&pString);
    string_destroy(&smaller);
}

Ref_Data* crypto_cache_read(void) {
    String* pString = file_get_string(crypto_cache_file_path);
    if (pString == NULL)
        return NULL;

    Json* jobj = json_tokener_parse(pString->data);
    if (jobj == NULL)
        return NULL;

    if (json_object_get_int64(json_object_object_get(json_object_object_get(jobj, "status"),
            "error_code")))
        return NULL;

    Ref_Data* pRef_Data = ref_data_init_length(json_object_array_length(json_object_object_get
            (jobj, "data")));
    ref_data_store_json_cmc(pRef_Data, jobj);

    json_object_put(jobj);
    string_destroy(&pString);
    return pRef_Data;
}

void ref_data_store_json_iex(Ref_Data* pRef_Data, const Json* jobj) {
    Json* idx;
    for (size_t i = 0; i < pRef_Data->length; i++) {
        idx = json_object_array_get_idx(jobj, i);
        strcpy(pRef_Data->slugs[i], json_object_get_string(json_object_object_get(idx,
                                                                                    "symbol")));
        strncpy(pRef_Data->names[i], json_object_get_string(json_object_object_get(idx, "name")),
                NAME_MAX_LENGTH -1);
        pRef_Data->names[i][NAME_MAX_LENGTH - 1] = '\0';
    }

    idx = json_object_array_get_idx(jobj, 1);

    // Date format: yyyy-mm-dd
    const char* date = json_object_get_string(json_object_object_get(idx, "date"));
    char year[5] = {0}, month[3] = {0}, day[3] = {0};
    strncpy(year, date, 4);
    strncpy(month, &date[5], 2);
    strncpy(day, &date[8], 2);
    struct tm time = {
            .tm_year = atoi(year) - 1900,
            .tm_mon = atoi(month) - 1,
            .tm_mday = atoi(day)
    };
    pRef_Data->time_loaded = mktime(&time);
}

void ref_data_store_json_cmc(Ref_Data* pRef_Data, const Json* jobj) {
    char date[DATE_MAX_LENGTH];
    strcpy(date, json_object_get_string(json_object_object_get(json_object_object_get(jobj,
            "status"), "timestamp")));
    for (size_t i = 0; i < strlen(date); i++) {
        if (date[i] == 'T') {
            date[i] = '\0';
            break;
        }
    }
    struct tm time = {0};
    sscanf(date, "%d-%d-%d", &time.tm_year, &time.tm_mon, &time.tm_mday);
    time.tm_year -= 1900;
    time.tm_mon--;
    pRef_Data->time_loaded = mktime(&time);

    Json* data = json_object_object_get(jobj, "data"), * idx;
    for (size_t i = 0; i < pRef_Data->length; i++) {
        idx = json_object_array_get_idx(data, i);
        strcpy(pRef_Data->names[i], json_object_get_string(json_object_object_get(idx, "name")));
        strcpy(pRef_Data->slugs[i], json_object_get_string(json_object_object_get(idx, "slug")));
    }
}

void info_array_store_endpoints_json(Info_Array* pInfo_Array, const Json* jobj) {
    Json* jsecurity;
    for (size_t i = 0; i < pInfo_Array->length; i++) {
        jsecurity = json_object_object_get(jobj, pInfo_Array->array[i]->slug);
        if (jsecurity != NULL)
            info_store_endpoints_json(pInfo_Array->array[i], jsecurity);

        // IEX symbols and slugs are the same
        strcpy(pInfo_Array->array[i]->symbol, pInfo_Array->array[i]->slug);
    }
}

void info_store_endpoints_json(Info* pInfo, const Json* jslug) {
    Json* jquote, * jchart, * jcompany, * jstats, * jpeers, * jnews, * jearnings;
    pInfo->api_provider = API_PROVIDER_IEX;
    jquote = json_object_object_get(jslug, "quote");
    jchart = json_object_object_get(jslug, "chart");
    jcompany = json_object_object_get(jslug, "company");
    jstats = json_object_object_get(jslug, "stats");
    jpeers = json_object_object_get(jslug, "peers");
    jnews = json_object_object_get(jslug, "news");
    jearnings = json_object_object_get(jslug, "earnings");
    if (jquote != NULL)
        info_store_quote_json(pInfo, jquote);
    if (jchart != NULL)
        info_store_chart_json(pInfo, jchart);
    if (jcompany != NULL)
        info_store_company_json(pInfo, jcompany);
    if (jstats != NULL)
        info_store_stats_json(pInfo, jstats);
    if (jpeers != NULL)
        info_store_peers_json(pInfo, jpeers);
    if (jnews != NULL)
        info_store_news_json(pInfo, jnews);
    if (jearnings != NULL)
        info_store_earnings_json(pInfo, jearnings);
}

void info_store_quote_json(Info* pInfo, const Json* jquote) {
    if (json_object_get_int64(json_object_object_get(jquote, "extendedPriceTime")) >
        json_object_get_int64(json_object_object_get(jquote, "latestUpdate"))) {
        pInfo->price = json_object_get_double(json_object_object_get(jquote, "extendedPrice"));
        pInfo->intraday_time = json_object_get_int64(json_object_object_get(jquote,
                                                                            "extendedPriceTime")) / 1000;
    } else {
        pInfo->price = json_object_get_double(json_object_object_get(jquote, "latestPrice"));
        pInfo->intraday_time = json_object_get_int64(json_object_object_get(jquote, "latestUpdate")) /
                               1000;
    }
    pInfo->price_last_close = json_object_get_double(json_object_object_get(jquote, "previousClose"));
    if (pInfo->price_last_close == 0) // May be 0 over weekend
        pInfo->price_last_close = EMPTY;
    pInfo->marketcap = json_object_get_int64(json_object_object_get(jquote, "marketCap"));
    pInfo->volume_1d = json_object_get_int64(json_object_object_get(jquote, "latestVolume"));
    pInfo->pe_ratio = json_object_get_double(json_object_object_get(jquote, "peRatio"));
}

void info_store_chart_json(Info* pInfo, const Json* jchart) {
    free(pInfo->points);
    pInfo->num_points = (int) json_object_array_length(jchart);
    pInfo->points = calloc((size_t) pInfo->num_points, sizeof(double));
    pointer_alloc_check(pInfo->points);
    for (int i = 0; i < pInfo->num_points; i++)
        pInfo->points[i] = json_object_get_double(
                json_object_object_get(json_object_array_get_idx(jchart, i), "close"));
    if (pInfo->price_last_close == EMPTY) // May be 0 over weekend, so get last close from points array
        pInfo->price_last_close = pInfo->points[pInfo->num_points - 1];
    if (pInfo->num_points > 5)
        pInfo->price_7d = pInfo->points[pInfo->num_points - 5];
    if (pInfo->num_points > 21)
        pInfo->price_30d = pInfo->points[pInfo->num_points - 21];
    if (pInfo->num_points < 25) // 1 month api data
        pInfo->price_30d = pInfo->points[0];
}

void info_store_company_json(Info* pInfo, const Json* jcompany) {
    Json* jsymbol, * jname, * jindustry, * jwebsite, * jdescription, * jceo, * jtype, * jsector;
    jsymbol = json_object_object_get(jcompany, "symbol");
    jname = json_object_object_get(jcompany, "companyName");
    jindustry = json_object_object_get(jcompany, "industry");
    jwebsite = json_object_object_get(jcompany, "website");
    jdescription = json_object_object_get(jcompany, "description");
    jceo = json_object_object_get(jcompany, "CEO");
    jtype = json_object_object_get(jcompany, "issueType");
    jsector = json_object_object_get(jcompany, "sector");

    if (jsymbol != NULL)
        strcpy(pInfo->symbol, json_object_get_string(jsymbol));
    if (jname != NULL) {
        strncpy(pInfo->name, json_object_get_string(jname), NAME_MAX_LENGTH - 1);
        pInfo->name[NAME_MAX_LENGTH -1] = '\0';
    }
    if (jindustry != NULL)
        strcpy(pInfo->industry, json_object_get_string(jindustry));
    if (jwebsite != NULL)
        strcpy(pInfo->website, json_object_get_string(jwebsite));
    if (jdescription != NULL)
        strcpy(pInfo->description, json_object_get_string(jdescription));
    if (jceo != NULL)
        strcpy(pInfo->ceo, json_object_get_string(jceo));
    if (jtype != NULL)
        strcpy(pInfo->issue_type, json_object_get_string(jtype));
    if (jsector != NULL)
        strcpy(pInfo->sector, json_object_get_string(jsector));
}

void info_store_stats_json(Info* pInfo, const Json* jstats) {
    pInfo->div_yield = json_object_get_double(json_object_object_get(jstats, "dividendYield"));
    pInfo->revenue = json_object_get_int64(json_object_object_get(jstats, "revenue"));
    pInfo->gross_profit = json_object_get_int64(json_object_object_get(jstats, "grossProfit"));
    pInfo->cash = json_object_get_int64(json_object_object_get(jstats, "cash"));
    pInfo->debt = json_object_get_int64(json_object_object_get(jstats, "debt"));
}

void info_store_peers_json(Info* pInfo, const Json* jpeers) {
    size_t len = json_object_array_length(jpeers);
    if (len == 0)
        return;

    pInfo->peers = info_array_init_length(len);
    for (size_t i = 0; i < pInfo->peers->length; i++)
        strcpy(pInfo->peers->array[i]->slug, json_object_get_string(
                json_object_array_get_idx(jpeers, i)));

    api_store_info_array(pInfo->peers, DATA_LEVEL_CHECK);
}

void info_store_news_json(Info* pInfo, const Json* jnews) {
    Json* idx, * headline, * source, * date, * summary, * url, * related;
    size_t len = json_object_array_length(jnews);
    if (len < (unsigned) pInfo->num_articles)
        pInfo->num_articles = (int)len;

    pInfo->articles = malloc(sizeof(News*) * pInfo->num_articles);
    pointer_alloc_check(pInfo->articles);

    for (int i = 0; i < pInfo->num_articles; i++) {
        idx = json_object_array_get_idx(jnews, (size_t) i);
        headline = json_object_object_get(idx, "headline");
        source = json_object_object_get(idx, "source");
        date = json_object_object_get(idx, "datetime");
        summary = json_object_object_get(idx, "summary");
        url = json_object_object_get(idx, "url");
        related = json_object_object_get(idx, "related");

        /*
         * If two articles in a row are the same, change num_articles and break loop. This will
         * happen if there are not enough articles supplied by API.
         */
        if (i > 0 && headline != NULL &&
                streq(json_object_get_string(headline), pInfo->articles[i - 1]->headline)) {
            pInfo->num_articles = i;
            break;
        }

        pInfo->articles[i] = news_init();
        if (headline != NULL)
            strcpy(pInfo->articles[i]->headline, json_object_get_string(headline));
        if (source != NULL)
            strcpy(pInfo->articles[i]->source, json_object_get_string(source));
        if (date != NULL)
            strncpy(pInfo->articles[i]->date, json_object_get_string(date), 10);
        pInfo->articles[i]->date[10] = '\0'; // strncpy doesn't null terminate
        if (summary != NULL)
            strcpy(pInfo->articles[i]->summary, json_object_get_string(summary));
        strip_tags(pInfo->articles[i]->summary); // Summary will be html formatted, so strip tags
        if (url != NULL)
            strcpy(pInfo->articles[i]->url, json_object_get_string(url));
        if (related != NULL)
            strcpy(pInfo->articles[i]->related, json_object_get_string(related));
        int related_num = 0;
        for (size_t j = 0; j < strlen(pInfo->articles[i]->related); j++) { // List only first five related slugs
            if (pInfo->articles[i]->related[j] == ',')
                related_num++;
            if (related_num == 5) {
                pInfo->articles[i]->related[j] = '\0';
                break;
            }
        }
    }
}

void info_store_earnings_json(Info* pInfo, const Json* jearnings) {
    // ETFs don't report earnings
    if (!json_object_is_type(json_object_object_get(jearnings, "earnings"), json_type_array))
        return;

    size_t len = json_object_array_length(json_object_object_get(jearnings, "earnings"));
    Json* idx, * period, * end_date, * report_date;
    for (size_t i = 0; i < len; i++) {
        idx = json_object_array_get_idx(json_object_object_get(jearnings, "earnings"), i);
        pInfo->eps[i] = json_object_get_double(json_object_object_get(idx, "actualEPS"));
        pInfo->eps_year_ago[i] = json_object_get_double(json_object_object_get(idx, "yearAgo"));
        period = json_object_object_get(idx, "fiscalPeriod");
        end_date = json_object_object_get(idx, "fiscalEndDate");
        report_date = json_object_object_get(idx, "EPSReportDate");
        if (period != NULL)
            strcpy(pInfo->fiscal_period[i], json_object_get_string(period));
        else if (end_date != NULL)
            strcpy(pInfo->fiscal_period[i], json_object_get_string(end_date));
        else if (report_date != NULL)
            strcpy(pInfo->fiscal_period[i], json_object_get_string(report_date));
    }
}

Info* info_array_find_slug(const Info_Array* pInfo_Array, const char* slug) {
    for (size_t i = 0; i < pInfo_Array->length; i++)
        if (streq(slug, pInfo_Array->array[i]->slug))
            return pInfo_Array->array[i];

    return NULL;
}

int ref_data_get_index_from_slug_bsearch(const Ref_Data* pRef_Data, const char* slug,
                                         size_t left, size_t right) {
    if (right < left)
        return -1;

    size_t mid = left + (right - left) / 2;
    int cmp = strcmp(slug, pRef_Data->slugs[mid]);
    if (cmp == 0)
        return (int) mid;

    if (cmp > 0)
        return ref_data_get_index_from_slug_bsearch(pRef_Data, slug, mid + 1, right);

    return ref_data_get_index_from_slug_bsearch(pRef_Data, slug, left, mid - 1);
}

int ref_data_get_index_from_name_bsearch(const Ref_Data* pRef_Data, const char* name,
                                         size_t left, size_t right) {
    if (right < left)
        return -1;

    size_t mid = left + (right - left) / 2;
    int cmp = strcmp(name, pRef_Data->names[mid]);
    if (cmp == 0)
        return (int) mid;

    if (cmp > 0)
        return ref_data_get_index_from_name_bsearch(pRef_Data, name, mid + 1, right);

    return ref_data_get_index_from_name_bsearch(pRef_Data, name, left, mid - 1);
}

Info* info_array_find_slug_recursive(const Info_Array* pInfo_Array, const char* slug) {
    Info* pInfo = NULL;
    if (pInfo_Array == NULL)
        return NULL;

    pInfo = info_array_find_slug(pInfo_Array, slug);
    if (pInfo != NULL)
        return pInfo;

    for (size_t i = 0; i < pInfo_Array->length; i++) {
        pInfo = info_array_find_slug_recursive(pInfo_Array->array[i]->peers, slug);
        if (pInfo != NULL)
            return pInfo;
    }
    return NULL;
}

void info_chart_fill_empty(Info* pInfo, int trading_days) {
    // Realloc for number of trading days
    pInfo->points = realloc(pInfo->points, sizeof(double) * trading_days);
    pointer_alloc_check(pInfo->points);
    // Move points to end
    int difference = trading_days - pInfo->num_points;
    memmove(&pInfo->points[difference], pInfo->points, sizeof(double) * pInfo->num_points);
    for (int i = 0; i < difference; i++) // Initialize newly allocated bytes as EMPTY
        pInfo->points[i] = EMPTY;

    pInfo->num_points = trading_days;
}

void key_ring_destroy(Key_Ring** phKeys) {
    free(*phKeys);
    *phKeys = NULL;
}

void ref_data_destroy(Ref_Data** phRef_Data) {
    if (*phRef_Data == NULL)
        return;

    Ref_Data* pRef_data = *phRef_Data;
    for (size_t i = 0; i < pRef_data->length; i++) {
        free(pRef_data->slugs[i]);
        free(pRef_data->names[i]);
    }
    free(pRef_data->slugs);
    free(pRef_data->names);
    free(*phRef_Data);
    *phRef_Data = NULL;
}

void news_destroy(News** phNews) {
    if (*phNews == NULL)
        return;

    free(*phNews);
    *phNews = NULL;
}

void info_destroy(Info** phInfo) {
    if (*phInfo == NULL)
        return;

    Info* pInfo = *phInfo;
    free(pInfo->points);
    if (pInfo->articles != NULL)
        for (int i = 0; i < pInfo->num_articles; i++)
            news_destroy(&pInfo->articles[i]);

    if (pInfo->peers != NULL)
        info_array_destroy(&pInfo->peers);

    free(pInfo->articles);
    free(*phInfo);
    *phInfo = NULL;
}

void info_array_destroy(Info_Array** phInfo_Array) {
    if (*phInfo_Array == NULL)
        return;

    Info_Array* pInfo_Array = *phInfo_Array;
    for (size_t i = 0; i < pInfo_Array->length; i++)
        info_destroy(&pInfo_Array->array[i]);
    free(pInfo_Array->array);
    info_destroy(&pInfo_Array->totals);
    free(*phInfo_Array);
    *phInfo_Array = NULL;
}
