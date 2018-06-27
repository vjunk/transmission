#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "transmission.h"
#include "proxylist.h"
#include "utils.h"
#include "file.h"
#include "log.h"

typedef struct tr_proxy_entry
{
    char* url_mask;
    char* proxy_url;
} tr_proxy_entry;

struct tr_proxy_list
{
    char* proxyListText;
    tr_proxy_entry* proxyEntrys;
};

static char const* getKey(void) { return _("Proxy list:"); }

static int tr_countLines(char const* text, size_t len)
{
    int count = 0;

    while (len > 0)
    {
        char const* eol;

        ++count;

        eol = memchr(text, '\n', len);
        if (eol == NULL)
            break;

        len -= eol + 1 - text;
        text = eol + 1;
    }

    return count;
}

static tr_proxy_entry* tr_parseProxyListText(char* text, size_t len)
{
    tr_proxy_entry* entrys = NULL;
    int i = 0;
    int line_count;
    enum
    {
        TR_PL_START,
        TR_PL_MASK,
        TR_PL_SPACE,
        TR_PL_PROXY,
        TR_PL_SKIP
    } state = TR_PL_START;

    line_count = tr_countLines(text, len);
    entrys = tr_malloc0((line_count + 1) * sizeof(*entrys));
    TR_ASSERT(entrys != NULL);

    while (*text != '\0')
    {
        if (*text == '\n')
        {
            *text = 0;

            if (entrys[i].url_mask != NULL)
            {
                if (entrys[i].proxy_url == NULL)
                {
                    entrys[i].proxy_url = text;
                }

                ++i;
            }

            state = TR_PL_START;
        }
        else if ((state != TR_PL_SKIP) && (*text == '#'))
        {
            *text = 0;
            state = TR_PL_SKIP;
        }
        else if ((state == TR_PL_START) && !isspace(*text))
        {
            entrys[i].url_mask = text;
            state = TR_PL_MASK;
        }
        else if ((state == TR_PL_MASK) && isspace(*text))
        {
            *text = 0;
            state = TR_PL_SPACE;
        }
        else if ((state == TR_PL_SPACE) && !isspace(*text))
        {
            entrys[i].proxy_url = text;
            state = TR_PL_PROXY;
        }
        else if (state == TR_PL_PROXY && isspace(*text))
        {
            *text = 0;
            state = TR_PL_SKIP;
        }

        ++text;
    }

    if ((entrys[i].url_mask != NULL) && (entrys[i].proxy_url == NULL))
    {
        entrys[i].proxy_url = text;
    }

    return entrys;
}

tr_proxy_list* tr_loadProxyList (char const* filename)
{
    tr_proxy_list* proxy_list = NULL;
    char* text;
    size_t len;
    tr_proxy_entry* proxy_entrys;

    text = (char*)tr_loadFile(filename, &len, NULL);
    if (text == NULL)
    {
        tr_logAddNamedError(getKey(), "Error reading file: %s", filename);
        return proxy_list;
    }

    proxy_entrys = tr_parseProxyListText(text, len); 

    if (proxy_entrys == NULL || proxy_entrys[0].url_mask == NULL)
    {
        tr_logAddNamedDbg(getKey(), "No proxy entrys in %s", filename);
        tr_free(proxy_entrys);
        tr_free(text);
        return proxy_list;
    }

    proxy_list = tr_malloc0(sizeof(*proxy_list));
    TR_ASSERT(proxy_list != NULL);

    proxy_list->proxyListText = text;
    proxy_list->proxyEntrys = proxy_entrys;

    tr_logAddNamedDbg(getKey(), "Loaded proxy list: %s", filename);

    return proxy_list;
}

void tr_freeProxyList(tr_proxy_list* proxy_list)
{
    if (proxy_list != NULL)
    {
        tr_free(proxy_list->proxyEntrys);
        tr_free(proxy_list->proxyListText);
        tr_free(proxy_list);
    }
}

char const* tr_getProxyUrlFromList(tr_proxy_list const* proxy_list, char const* tracker_url)
{
    tr_proxy_entry const* proxy;

    TR_ASSERT(proxy_list != NULL);
    TR_ASSERT(tracker_url != NULL);

    /* This will be slow for large proxy list, so don't use large lists */
    for (proxy = proxy_list->proxyEntrys; proxy->url_mask != NULL; ++proxy)
    {
        if (tr_wildmat(tracker_url, proxy->url_mask))
        {
            return proxy->proxy_url;
        }
    }

    return NULL;
}

