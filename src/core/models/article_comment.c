#include "article_comment.h"
#include "src/core/helpers/gobject_helpers.h"

struct _ArticleComment {
    GObject parent_instance;
    gchar *user;
    gchar *date;
    gchar *text;
};

enum { PROP_0, PROP_USER, PROP_DATE, PROP_TEXT, N_PROPS };
static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE(
    ArticleComment,
    article_comment,
    G_TYPE_OBJECT
)

static void article_comment_get_property(
    GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *spec
) {
    ArticleComment *self = ARTICLE_COMMENT(object);

    switch (property_id) {
    case PROP_USER:
        g_value_set_string(value, self->user);
        break;
    case PROP_DATE:
        g_value_set_string(value, self->date);
        break;
    case PROP_TEXT:
        g_value_set_string(value, self->text);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void article_comment_set_property(
    GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *spec
) {
    ArticleComment *self = ARTICLE_COMMENT(object);

    gchar **field = NULL;
    switch (property_id) {
    case PROP_USER:
        field = &self->user;
        break;
    case PROP_DATE:
        field = &self->date;
        break;
    case PROP_TEXT:
        field = &self->text;
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
        return;
    }

    g_free(*field);
    *field = g_value_dup_string(value);
}

static void article_comment_finalize(GObject *object) {
    ArticleComment *self = ARTICLE_COMMENT(object);
    g_free(self->user);
    g_free(self->date);
    g_free(self->text);
    G_OBJECT_CLASS(article_comment_parent_class)->finalize(object);
}

static void article_comment_class_init(ArticleCommentClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = article_comment_get_property;
    object_class->set_property = article_comment_set_property;
    object_class->finalize = article_comment_finalize;

    properties[PROP_USER] = gobject_helpers_string_property("user");
    properties[PROP_DATE] = gobject_helpers_string_property("date");
    properties[PROP_TEXT] = gobject_helpers_string_property("text");
    g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void article_comment_init(ArticleComment *self) { (void)self; }

ArticleComment *article_comment_new(
    const gchar *user,
    const gchar *date,
    const gchar *text
) {
    return g_object_new(
        ARTICLE_TYPE_COMMENT,
        "user",
        user,
        "date",
        date,
        "text",
        text,
        NULL
    );
}

const gchar *article_comment_get_user(ArticleComment *self) {
    return self->user;
}

const gchar *article_comment_get_date(ArticleComment *self) {
    return self->date;
}

const gchar *article_comment_get_text(ArticleComment *self) {
    return self->text;
}
