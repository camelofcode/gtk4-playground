#include "article_detail.h"

#include "src/core/helpers/gobject_helpers.h"
#include "src/core/helpers/json_helpers.h"

struct _ArticleDetail {
    GObject parent_instance;
    gchar *id;
    gchar *name;
    gchar *image;
    gchar *date;
    gchar *description;
    GListStore *comments;
};

enum {
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_IMAGE,
    PROP_DATE,
    PROP_DESCRIPTION,
    PROP_COMMENTS,
    N_PROPS
};
static GParamSpec *properties[N_PROPS];

static void article_detail_serializable_init(JsonSerializableIface *iface);
G_DEFINE_TYPE_WITH_CODE(
    ArticleDetail,
    article_detail,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        JSON_TYPE_SERIALIZABLE,
        article_detail_serializable_init
    )
)

static void article_detail_get_property(
    GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *spec
) {
    ArticleDetail *self = ARTICLE_DETAIL(object);

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
    case PROP_DESCRIPTION:
        g_value_set_string(value, self->description);
        break;
    case PROP_COMMENTS:
        g_value_set_object(value, self->comments);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void article_detail_set_property(
    GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *spec
) {
    ArticleDetail *self = ARTICLE_DETAIL(object);

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
    case PROP_DESCRIPTION:
        field = &self->description;
        break;
    case PROP_COMMENTS:
        g_set_object(&self->comments, g_value_get_object(value));
        return;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
        return;
    }

    g_free(*field);
    *field = g_value_dup_string(value);
}

static void article_detail_finalize(GObject *object) {
    ArticleDetail *self = ARTICLE_DETAIL(object);
    g_free(self->id);
    g_free(self->name);
    g_free(self->image);
    g_free(self->date);
    g_free(self->description);
    g_clear_object(&self->comments);
    G_OBJECT_CLASS(article_detail_parent_class)->finalize(object);
}

static void article_detail_class_init(ArticleDetailClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = article_detail_get_property;
    object_class->set_property = article_detail_set_property;
    object_class->finalize = article_detail_finalize;

    properties[PROP_ID] = gobject_helpers_string_property("id");
    properties[PROP_NAME] = gobject_helpers_string_property("name");
    properties[PROP_IMAGE] = gobject_helpers_string_property("image");
    properties[PROP_DATE] = gobject_helpers_string_property("date");
    properties[PROP_DESCRIPTION] =
        gobject_helpers_string_property("description");
    properties[PROP_COMMENTS] = gobject_helpers_list_store_property("comments");
    g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void article_detail_init(ArticleDetail *self) {
    self->comments = g_list_store_new(ARTICLE_TYPE_COMMENT);
}

static gboolean article_detail_deserialize_property(
    JsonSerializable *object,
    const gchar *name,
    GValue *value,
    GParamSpec *spec,
    JsonNode *node
) {
    if (g_str_equal(name, "comments")) {
        GListStore *comments =
            json_helpers_deserialize_list(node, ARTICLE_TYPE_COMMENT, NULL);
        if (!comments) {
            return FALSE;
        }
        g_value_take_object(value, comments);
        return TRUE;
    }

    return json_serializable_default_deserialize_property(
        object,
        name,
        value,
        spec,
        node
    );
}

static void article_detail_serializable_init(JsonSerializableIface *iface) {
    iface->deserialize_property = article_detail_deserialize_property;
}

ArticleDetail *article_detail_new(
    const gchar *id,
    const gchar *name,
    const gchar *image,
    const gchar *date,
    const gchar *description
) {
    return g_object_new(
        ARTICLE_TYPE_DETAIL,
        "id",
        id,
        "name",
        name,
        "image",
        image,
        "date",
        date,
        "description",
        description,
        NULL
    );
}

const gchar *article_detail_get_id(ArticleDetail *self) { return self->id; }
const gchar *article_detail_get_name(ArticleDetail *self) { return self->name; }
const gchar *article_detail_get_image(ArticleDetail *self) {
    return self->image;
}
const gchar *article_detail_get_date(ArticleDetail *self) { return self->date; }
const gchar *article_detail_get_description(ArticleDetail *self) {
    return self->description;
}
GListModel *article_detail_get_comments(ArticleDetail *self) {
    return G_LIST_MODEL(self->comments);
}
