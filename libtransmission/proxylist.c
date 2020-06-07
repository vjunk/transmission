#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "transmission.h"
#include "proxylist.h"
#include "variant.h"
#include "utils.h"
#include "file.h"
#include "log.h"
#include "tr-assert.h"

static char const* getKey(void)
{
    return "Proxy list:";
}

static int tr_countLines(char const* text, size_t len)
{
    int count = 0;

    while (len > 0)
    {
        char const* eol;

        ++count;

        eol = memchr(text, '\n', len);
        if (eol == NULL)
        {
            break;
        }

        len -= eol + 1 - text;
        text = eol + 1;
    }

    return count;
}

static void tr_parseProxyListText(tr_variant* setme, char* text, size_t len)
{
    int line_count;
    char* url_mask = NULL;
    char* proxy_url = NULL;
    enum
    {
        TR_PL_START,
        TR_PL_MASK,
        TR_PL_SPACE,
        TR_PL_PROXY,
        TR_PL_SKIP
    }
    state = TR_PL_START;

    line_count = tr_countLines(text, len);

    tr_variantFree(setme);
    tr_variantInitList(setme, line_count);

    while (*text != '\0')
    {
        if (*text == '\n')
        {
            *text = 0;

            if (url_mask != NULL)
            {
                if (proxy_url == NULL)
                {
                    proxy_url = text;
                }

                tr_variantListAddStr(setme, url_mask);
                tr_variantListAddStr(setme, proxy_url);
            }

            state = TR_PL_START;
            url_mask = NULL;
            proxy_url = NULL;
        }
        else if ((state != TR_PL_SKIP) && (*text == '#'))
        {
            *text = 0;
            state = TR_PL_SKIP;
        }
        else if ((state == TR_PL_START) && !isspace(*text))
        {
            url_mask = text;
            state = TR_PL_MASK;
        }
        else if ((state == TR_PL_MASK) && isspace(*text))
        {
            *text = 0;
            state = TR_PL_SPACE;
        }
        else if ((state == TR_PL_SPACE) && !isspace(*text))
        {
            proxy_url = text;
            state = TR_PL_PROXY;
        }
        else if (state == TR_PL_PROXY && isspace(*text))
        {
            *text = 0;
            state = TR_PL_SKIP;
        }

        ++text;
    }

    if (url_mask != NULL)
    {
        if (proxy_url == NULL)
        {
            proxy_url = text;
        }

        tr_variantListAddStr(setme, url_mask);
        tr_variantListAddStr(setme, proxy_url);
    }
}

tr_proxy_list* tr_loadProxyList(char const* filename)
{
    tr_proxy_list* proxy_list;
    char* text;
    size_t len;

    proxy_list = tr_malloc0(sizeof(*proxy_list));
    TR_ASSERT(proxy_list != NULL);
    tr_variantInitList(proxy_list, 0);

    text = (char*)tr_loadFile(filename, &len, NULL);
    if (text == NULL)
    {
        tr_logAddNamedError(getKey(), "Error reading file: %s", filename);
        return proxy_list;
    }

    tr_parseProxyListText(proxy_list, text, len);
    tr_free(text);

    if (tr_variantListSize(proxy_list) / 2 == 0)
    {
        tr_logAddNamedDbg(getKey(), "No proxy entrys in %s", filename);
    }

    tr_logAddNamedDbg(getKey(), "Loaded proxy list: %s", filename);

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

char const* tr_getProxyUrlFromList(tr_proxy_list* proxy_list, char const* tracker_url)
{
    size_t sz;
    size_t i;

    TR_ASSERT(tr_variantIsList(proxy_list));
    TR_ASSERT(tracker_url != NULL);

    sz = tr_variantListSize(proxy_list);

    /* This will be slow for large proxy list, so don't use large lists */
    for (i = 0; i < sz; i += 2)
    {
        char const* url_mask;
        char const* proxy_url;

        if (tr_variantGetStr(tr_variantListChild(proxy_list, i), &url_mask, NULL)
            && tr_variantGetStr(tr_variantListChild(proxy_list, i + 1), &proxy_url, NULL))
        {
            if (tr_wildmat(tracker_url, url_mask))
            {
                return proxy_url;
            }
        }
    }

    return NULL;
}
