#include <ctype.h>
#include <string.h> /* memcpy, memset, memcmp */
#include "transmission.h"
#include "proxylist.h"
#include "variant.h"
#include "utils.h"
#include "tr-assert.h"

void tr_proxyTextToVarList(tr_variant *vlist, char const* text)
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

    TR_ASSERT(tr_variantIsList(vlist));

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

                tr_variantListAddRaw(vlist, url_mask, url_mask_end - url_mask);
                tr_variantListAddRaw(vlist, proxy_url, proxy_url_end - proxy_url);
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

static size_t countTextSize(tr_variant const* vlist, size_t* align)
{
    size_t count = tr_variantListSize(vlist);
    size_t len1 = 0;
    size_t len2 = 0;
    size_t i;

    for (i = 1; i < count; i += 2)
    {
        size_t len;
        char const* str;
        tr_variant* vstr;

        vstr = tr_variantListChild((tr_variant *)vlist, i - 1);
        tr_variantGetStr(vstr, &str, &len);
        if (len1 < len)
        {
            len1 = len;
        }

        vstr = tr_variantListChild((tr_variant *)vlist, i);
        tr_variantGetStr(vstr, &str, &len);
        len2 += len;
    }

    count = count / 2;

    *align = len1;
    return (len1 + 2) * count + len2 + 1;
}

char* tr_proxyVarListToText(tr_variant const* vlist)
{
    size_t size;
    size_t align;
    size_t count;
    size_t i;
    char* buf;
    char* pos;

    if (vlist == NULL)
    {
        count = 0;
        size = 1;
        align = 0;
    }
    else
    {
        count = tr_variantListSize(vlist);
        size = countTextSize(vlist, &align);
    }

    buf = tr_malloc0(size);
    TR_ASSERT(buf != NULL);
    memset(buf, ' ', size - 1);
    pos = buf;

    for (i = 1; i < count; i += 2)
    {
        size_t len;
        char const* str;
        tr_variant* vstr;

        vstr = tr_variantListChild((tr_variant *)vlist, i - 1);
        tr_variantGetStr(vstr, &str, &len);
        memcpy(pos, str, len);
        pos += align;

        ++pos; /* One space */

        vstr = tr_variantListChild((tr_variant *)vlist, i);
        tr_variantGetStr(vstr, &str, &len);
        memcpy(pos, str, len);
        pos += len;

        *pos++ = '\n';
    }

    *pos = '\0';

    return buf;
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

