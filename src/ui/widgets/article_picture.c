#include "article_picture.h"

typedef struct {
    GWeakRef picture;
    guint generation;
} PictureRequest;

typedef struct {
    GCancellable *cancellable;
    guint generation;
} PictureState;

static const gchar PICTURE_STATE_KEY[] = "article-picture-state";

static void picture_state_free(gpointer pointer) {
    PictureState *state = pointer;
    if (state->cancellable) {
        g_cancellable_cancel(state->cancellable);
    }
    g_clear_object(&state->cancellable);
    g_free(state);
}

static PictureState *get_picture_state(GtkPicture *picture) {
    PictureState *state =
        g_object_get_data(G_OBJECT(picture), PICTURE_STATE_KEY);
    if (!state) {
        state = g_new0(PictureState, 1);
        g_object_set_data_full(
            G_OBJECT(picture),
            PICTURE_STATE_KEY,
            state,
            picture_state_free
        );
    }
    return state;
}

static void picture_request_free(PictureRequest *request) {
    g_weak_ref_clear(&request->picture);
    g_free(request);
}

static void picture_loaded(
    ArticlesApi *api,
    GdkTexture *texture,
    const GError *error,
    gpointer user_data
) {
    (void)api;
    (void)error;

    PictureRequest *request = user_data;

    g_autoptr(GtkPicture) picture = g_weak_ref_get(&request->picture);
    if (!picture || gtk_widget_in_destruction(GTK_WIDGET(picture))) {
        picture_request_free(request);
        return;
    }

    PictureState *state =
        g_object_get_data(G_OBJECT(picture), PICTURE_STATE_KEY);
    if (!state || state->generation != request->generation) {
        picture_request_free(request);
        return;
    }

    g_clear_object(&state->cancellable);
    if (texture) {
        gtk_picture_set_paintable(picture, GDK_PAINTABLE(texture));
    }

    picture_request_free(request);
}

void article_picture_load(
    GtkPicture *picture,
    ArticlesApi *api,
    const gchar *url
) {
    g_return_if_fail(GTK_IS_PICTURE(picture));
    g_return_if_fail(ARTICLES_IS_API(api));

    PictureState *state = get_picture_state(picture);
    if (state->cancellable) {
        g_cancellable_cancel(state->cancellable);
    }
    g_clear_object(&state->cancellable);
    state->generation++;
    gtk_picture_set_paintable(picture, NULL);

    if (!url || !*url) {
        return;
    }

    state->cancellable = g_cancellable_new();
    PictureRequest *request = g_new0(PictureRequest, 1);
    g_weak_ref_init(&request->picture, picture);
    request->generation = state->generation;
    articles_api_download_image_async(
        api,
        url,
        state->cancellable,
        picture_loaded,
        request
    );
}

void article_picture_cancel(GtkPicture *picture) {
    g_return_if_fail(GTK_IS_PICTURE(picture));

    PictureState *state =
        g_object_get_data(G_OBJECT(picture), PICTURE_STATE_KEY);
    if (!state) {
        return;
    }

    if (state->cancellable) {
        g_cancellable_cancel(state->cancellable);
    }
    g_clear_object(&state->cancellable);
    state->generation++;
}
