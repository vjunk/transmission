#include <string.h> /* memcmp */
#include "transmission.h"
#include "proxylist.h"
#include "variant.h"
#include "utils.h"
#include "tr-assert.h"

tr_proxy_list* tr_proxyListNew(void)
{
    tr_proxy_list* proxy_list = tr_malloc0(sizeof(*proxy_list));
    TR_ASSERT(proxy_list != NULL);
    tr_variantInitList(proxy_list, 0);

    return proxy_list;
}

void tr_proxyListFree(tr_proxy_list* proxy_list)
{
    if (proxy_list != NULL)
    {
        tr_variantFree(proxy_list);
        tr_free(proxy_list);
    }
}

void tr_proxyListUpdate(tr_proxy_list* proxy_list, tr_variant const* vlist)
{
    TR_ASSERT(tr_variantIsList(proxy_list));
    tr_variantFree(proxy_list);
    tr_variantInitList(proxy_list, 0);

    if(vlist == NULL)
    {
        return;
    }

    TR_ASSERT(tr_variantIsList(proxy_list));
    tr_variantListCopy(proxy_list, vlist);
}

void tr_proxyListGet(tr_proxy_list* const proxy_list, tr_variant* vlist)
{
    TR_ASSERT(tr_variantIsList(vlist));
    tr_variantFree(vlist);
    tr_variantInitList(vlist, 0);

    if(proxy_list == NULL)
    {
        return;
    }

    TR_ASSERT(tr_variantIsList(proxy_list));
    tr_variantListCopy(vlist, proxy_list);
}

char const* tr_proxyGetUrl(tr_proxy_list const* proxy_list, char const* tracker_url)
{
    size_t count;
    size_t i;

    if (proxy_list == NULL)
    {
        return NULL;
    }

    TR_ASSERT(tr_variantIsList(proxy_list));
    TR_ASSERT(tracker_url != NULL);

    count = tr_variantListSize(proxy_list);

    /* This will be slow for large proxy list, so don't use large lists */
    for (i = 1; i < count; i += 2)
    {
        char const* url_mask;
        char const* proxy_url;

        if (tr_variantGetStr(tr_variantListChild((tr_variant*)proxy_list, i - 1), &url_mask, NULL)
            && tr_variantGetStr(tr_variantListChild((tr_variant*)proxy_list, i), &proxy_url, NULL))
        {
            if (tr_wildmat(tracker_url, url_mask))
            {
                return proxy_url;
            }
        }
    }

    return NULL;
}

bool tr_proxyCompareVarLists(tr_variant const* a, tr_variant const* b)
{
    size_t count;
    size_t i;

    if (a == NULL && b == NULL)
    {
        return true;
    }

    if (a == NULL || b == NULL)
    {
        return false;
    }

    TR_ASSERT(tr_variantIsList(a));
    TR_ASSERT(tr_variantIsList(b));

    count = tr_variantListSize(a);

    if (tr_variantListSize(b) != count)
    {
        return false;
    }

    for (i = 0; i < count; ++i)
    {
        size_t len_a;
        size_t len_b;
        char const* str_a;
        char const* str_b;
        tr_variant* vstr;

        vstr = tr_variantListChild((tr_variant *)a, i);
        tr_variantGetStr(vstr, &str_a, &len_a);

        vstr = tr_variantListChild((tr_variant *)b, i);
        tr_variantGetStr(vstr, &str_b, &len_b);

        if (len_a != len_b)
        {
            return false;
        }

        if (memcmp(str_a, str_b, len_a) != 0)
        {
            return false;
        }
    }

    return true;
}

