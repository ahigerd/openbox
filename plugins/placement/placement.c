#include "kernel/dispatch.h"
#include "kernel/client.h"
#include "kernel/frame.h"
#include "kernel/screen.h"
#include "kernel/openbox.h"
#include "kernel/parse.h"
#include "history.h"
#include <glib.h>

static gboolean history;

static void parse_assign(char *name, ParseToken *value)
{
    if  (!g_ascii_strcasecmp(name, "remember")) {
        if (value->type != TOKEN_BOOL)
            yyerror("invalid value");
        else
            history = value->data.bool;
    } else
        yyerror("invalid option");
    parse_free_token(value);
}

void plugin_setup_config()
{
    history = TRUE;

    parse_reg_section("placement", NULL, parse_assign);
}

static void place_random(Client *c)
{
    int l, r, t, b;
    int x, y;
    Rect *area;

    if (ob_state == State_Starting) return;

    area = screen_area(c->desktop);

    l = area->x;
    t = area->y;
    r = area->x + area->width - c->frame->area.width;
    b = area->y + area->height - c->frame->area.height;

    if (r > l) x = g_random_int_range(l, r + 1);
    else       x = 0;
    if (b > t) y = g_random_int_range(t, b + 1);
    else       y = 0;

    frame_frame_gravity(c->frame, &x, &y); /* get where the client should be */
    client_configure(c, Corner_TopLeft, x, y, c->area.width, c->area.height,
                     TRUE, TRUE);
}

static void event(ObEvent *e, void *foo)
{
    g_assert(e->type == Event_Client_New);

    /* requested a position */
    if (e->data.c.client->positioned) return;

    if (!history || !place_history(e->data.c.client))
        place_random(e->data.c.client);
}

void plugin_startup()
{
    dispatch_register(Event_Client_New, (EventHandler)event, NULL);

    history_startup();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);

    history_shutdown();
}
