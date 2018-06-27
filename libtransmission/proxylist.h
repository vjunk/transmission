#pragma once

#ifndef __TRANSMISSION__
#   error only libtransmission should #include this header.
#endif

typedef struct tr_proxy_list tr_proxy_list;

tr_proxy_list* tr_loadProxyList(char const* filename);

void tr_freeProxyList(tr_proxy_list* proxy_list);

char const* tr_getProxyUrlFromList(tr_proxy_list const* proxy_list, char const* tracker_url);
