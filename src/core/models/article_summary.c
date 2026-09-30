#include "article_summary.h"
#include "src/core/helpers/gobject_helpers.h"

struct _ArticleSummary {
    GObject parent_instance;
    gchar *id;
    gchar *name;
    gchar *image;
    gchar *date;
};

enum { PROP_0, PROP_ID, PROP_NAME, PROP_IMAGE, PROP_DATE, N_PROPS };
static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE(
    ArticleSummary,
    article_summary,
    G_TYPE_OBJECT
)

static void article_summary_get_property(
    GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *spec
) {
    ArticleSummary *self = ARTICLE_SUMMARY(object);

    switch (property_id) {
    case PROP_ID:
        g_value_set_string(value, self->id);
        break;
    case PROP_NAME:
        g_value_set_string(value, self->name);
        break;
    case PROP_IMAGE:
        g_value_set_string(value, self->image);
        break;
    case PROP_DATE:
        g_value_set_string(value, self->date);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void article_summary_set_property(
    GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *spec
) {
    ArticleSummary *self = ARTICLE_SUMMARY(object);

    gchar **field = NULL;
    switch (property_id) {
    case PROP_ID:
        field = &self->id;
        break;
    case PROP_NAME:
        field = &self->name;
        break;
    case PROP_IMAGE:
        field = &self->image;
        break;
    case PROP_DATE:
        field = &self->date;
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
        return;
    }

    g_free(*field);
    *field = g_value_dup_string(value);
}

static void article_summary_finalize(GObject *object) {
    ArticleSummary *self = ARTICLE_SUMMARY(object);
    g_free(self->id);
    g_free(self->name);
    g_free(self->image);
    g_free(self->date);
    G_OBJECT_CLASS(article_summary_parent_class)->finalize(object);
}

static void article_summary_class_init(ArticleSummaryClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = article_summary_get_property;
    object_class->set_property = article_summary_set_property;
    object_class->finalize = article_summary_finalize;

    properties[PROP_ID] = gobject_helpers_string_property("id");
    properties[PROP_NAME] = gobject_helpers_string_property("name");
    properties[PROP_IMAGE] = gobject_helpers_string_property("image");
    properties[PROP_DATE] = gobject_helpers_string_property("date");
    g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void article_summary_init(ArticleSummary *self) { (void)self; }

ArticleSummary *article_summary_new(
    const gchar *id,
    const gchar *name,
    const gchar *image,
    const gchar *date
) {
    return g_object_new(
        ARTICLE_TYPE_SUMMARY,
        "id",
        id,
        "name",
        name,
        "image",
        image,
        "date",
        date,
        NULL
    );
}

const gchar *article_summary_get_id(ArticleSummary *self) { return self->id; }

const gchar *article_summary_get_name(ArticleSummary *self) {
    return self->name;
}

const gchar *article_summary_get_image(ArticleSummary *self) {
    return self->image;
}

const gchar *article_summary_get_date(ArticleSummary *self) {
    return self->date;
}
