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

    if (vlist == NULL)
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

    if (proxy_list == NULL)
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

        if (tr_variantGetStr(tr_variantListChild((tr_variant*)proxy_list, i - 1), &url_mask, NULL) &&
            tr_variantGetStr(tr_variantListChild((tr_variant*)proxy_list, i), &proxy_url, NULL))
        {
            if (tr_wildmat(tracker_url, url_mask))
            {
                return proxy_url;
            }
        }
    }

    return NULL;
}
