#include <ctype.h>
#include "transmission.h"
#include "proxylist.h"
#include "variant.h"
#include "utils.h"
#include "tr-assert.h"

void tr_parseProxyListText(tr_proxy_list* proxy_list, char const* text)
{
    char const* url_mask = NULL;
    char const* url_mask_end = NULL;
    char const* proxy_url = NULL;
    char const* proxy_url_end = NULL;
    enum
    {
        TR_PL_START,
        TR_PL_MASK,
        TR_PL_SPACE,
        TR_PL_PROXY,
        TR_PL_SKIP,
        TR_PL_FINISH
    }
    state = TR_PL_START;

    TR_ASSERT(tr_variantIsList(proxy_list));

    tr_variantFree(proxy_list);
    tr_variantInitList(proxy_list, 0);

    while (state != TR_PL_FINISH)
    {
        if ((*text == '\n') || (*text == '\0'))
        {
            if (url_mask != NULL)
            {
                if (url_mask_end == NULL)
                {
                    url_mask_end = text;
                }

                if (proxy_url == NULL)
                {
                    proxy_url = text;
                }

                if (proxy_url_end == NULL)
                {
                    proxy_url_end = text;
                }

                tr_variantListAddRaw(proxy_list, url_mask, url_mask_end - url_mask);
                tr_variantListAddRaw(proxy_list, proxy_url, proxy_url_end - proxy_url);
            }

            if (*text == '\0')
            {
                state = TR_PL_FINISH;
            }
            else
            {
                state = TR_PL_START;
                url_mask = NULL;
                url_mask_end = NULL;
                proxy_url = NULL;
                proxy_url_end = NULL;
            }
        }
        else if (*text == '#')
        {
            if (state == TR_PL_MASK)
            {
                url_mask_end = text;
            }
            else if (state == TR_PL_PROXY)
            {
                proxy_url_end = text;
            }

            state = TR_PL_SKIP;
        }
        else if ((state == TR_PL_START) && !isspace(*text))
        {
            url_mask = text;
            state = TR_PL_MASK;
        }
        else if ((state == TR_PL_MASK) && isspace(*text))
        {
            url_mask_end = text;
            state = TR_PL_SPACE;
        }
        else if ((state == TR_PL_SPACE) && !isspace(*text))
        {
            proxy_url = text;
            state = TR_PL_PROXY;
        }
        else if ((state == TR_PL_PROXY) && isspace(*text))
        {
            proxy_url_end = text;
            state = TR_PL_SKIP;
        }

        ++text;
    }
}

void tr_copyProxyList(tr_proxy_list* target, tr_proxy_list const* source)
{
    size_t count;
    size_t i;

    TR_ASSERT(target != NULL);
    tr_variantFree(target);

    if (source == NULL)
    {
        tr_variantInitList(target, 0);
        return;
    }

    TR_ASSERT(tr_variantIsList(source));

    count = tr_variantListSize(source);
    tr_variantInitList(target, count);

    for (i = 0; i < count; ++i)
    {
        size_t len;
        char const* str;
        tr_variant* vstr = tr_variantListChild((tr_variant *)source, i);

        tr_variantGetStr(vstr, &str, &len);
        tr_variantListAddRaw(target, str, len);
    }
}

tr_proxy_list* tr_cloneProxyList(tr_proxy_list const* source)
{
    tr_proxy_list* proxy_list;

    TR_ASSERT(tr_variantIsList(source));

    proxy_list = tr_malloc0(sizeof(*proxy_list));
    TR_ASSERT(proxy_list != NULL);
    tr_variantInitList(proxy_list, 0);
    tr_copyProxyList(proxy_list, source);

    return proxy_list;
}

void tr_freeProxyList(tr_proxy_list* proxy_list)
{
    if (proxy_list != NULL)
    {
        tr_variantFree(proxy_list);
        tr_free(proxy_list);
    }
}

char const* tr_getProxyUrlFromList(tr_proxy_list const* proxy_list, char const* tracker_url)
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
