#pragma once

#ifndef __TRANSMISSION__
#   error only libtransmission should #include this header.
#endif

#include "variant.h"

typedef tr_variant tr_proxy_list;

void tr_parseProxyListText(tr_proxy_list* proxy_list, char const* text);

void tr_copyProxyList(tr_proxy_list* target, tr_proxy_list const* source);

tr_proxy_list* tr_cloneProxyList(tr_proxy_list const* source);

void tr_freeProxyList(tr_proxy_list* proxy_list);

char const* tr_getProxyUrlFromList(tr_proxy_list const* proxy_list, char const* tracker_url);
