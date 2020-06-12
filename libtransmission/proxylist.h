#pragma once

#ifndef __TRANSMISSION__
#   error only libtransmission should #include this header.
#endif

#include "variant.h"

typedef tr_variant tr_proxy_list;

tr_proxy_list* tr_proxyListNew(void);

void tr_proxyListFree(tr_proxy_list* proxy_list);

void tr_proxyListUpdate(tr_proxy_list* proxy_list, tr_variant const* vlist);

void tr_proxyListGet(tr_proxy_list* const proxy_list, tr_variant* vlist);

char const* tr_proxyGetUrl(tr_proxy_list const* proxy_list, char const* tracker_url);
