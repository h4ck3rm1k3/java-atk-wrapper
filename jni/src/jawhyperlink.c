/*
 * Java ATK Wrapper for GNOME
 * Copyright (C) 2009 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <glib.h>
#include "jawhyperlink.h"
#include "jawutil.h"
#include "jawimpl.h"

static void			jaw_hyperlink_class_init		(JawHyperlinkClass	*klass);
static void			jaw_hyperlink_dispose			(GObject		*gobject);
static void			jaw_hyperlink_finalize			(GObject		*gobject);

/* AtkObject */
static gchar*			jaw_hyperlink_get_uri			(AtkHyperlink		*atk_hyperlink,
									 gint			i);
static AtkObject*		jaw_hyperlink_get_object		(AtkHyperlink		*atk_hyperlink,
									 gint			i);
static gint			jaw_hyperlink_get_end_index		(AtkHyperlink		*atk_hyperlink);
static gint			jaw_hyperlink_get_start_index		(AtkHyperlink		*atk_hyperlink);
static gboolean			jaw_hyperlink_is_valid			(AtkHyperlink		*atk_hyperlink);
static gint			jaw_hyperlink_get_n_anchors		(AtkHyperlink		*atk_hyperlink);

G_DEFINE_TYPE (JawHyperlink, jaw_hyperlink, ATK_TYPE_HYPERLINK)

JawHyperlink*
jaw_hyperlink_new (jobject jhyperlink)
{
	JawHyperlink* jaw_hyperlink = g_object_new(JAW_TYPE_HYPERLINK, NULL);
	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jaw_hyperlink->jhyperlink = (*jniEnv)->NewGlobalRef(jniEnv, jhyperlink);

	return jaw_hyperlink;
}

static void
jaw_hyperlink_class_init (JawHyperlinkClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = jaw_hyperlink_dispose;
	gobject_class->finalize = jaw_hyperlink_finalize;

	AtkHyperlinkClass *atk_hyperlink_class = ATK_HYPERLINK_CLASS(klass);
	atk_hyperlink_class->get_uri = jaw_hyperlink_get_uri;
	atk_hyperlink_class->get_object = jaw_hyperlink_get_object;
	atk_hyperlink_class->get_end_index = jaw_hyperlink_get_end_index;
	atk_hyperlink_class->get_start_index = jaw_hyperlink_get_start_index;
	atk_hyperlink_class->is_valid = jaw_hyperlink_is_valid;
	atk_hyperlink_class->get_n_anchors = jaw_hyperlink_get_n_anchors;
}

static void
jaw_hyperlink_init (JawHyperlink *link)
{
}

static void
jaw_hyperlink_dispose(GObject *gobject)
{
	/* Chain up to parent's dispose */
	G_OBJECT_CLASS(jaw_hyperlink_parent_class)->dispose(gobject);
}

static void
jaw_hyperlink_finalize(GObject *gobject)
{
	JawHyperlink *jaw_hyperlink = JAW_HYPERLINK(gobject);

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	(*jniEnv)->DeleteGlobalRef(jniEnv, jaw_hyperlink->jhyperlink);
	jaw_hyperlink->jhyperlink = NULL;

	/* Chain up to parent's finalize */
	G_OBJECT_CLASS(jaw_hyperlink_parent_class)->finalize(gobject);
}

static gchar*
jaw_hyperlink_get_uri (AtkHyperlink *atk_hyperlink,
			gint i)
{
	JawHyperlink *jaw_hyperlink = JAW_HYPERLINK(atk_hyperlink);
	jobject jhyperlink = jaw_hyperlink->jhyperlink;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHyperlink = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHyperlink, "get_uri", "(I)Ljava/lang/String;");
	jstring jstr = (*jniEnv)->CallObjectMethod(jniEnv, jhyperlink, jmid, (jint)i);
	
	if (jaw_hyperlink->uri != NULL) {
		(*jniEnv)->ReleaseStringUTFChars(jniEnv, jaw_hyperlink->jstrUri, jaw_hyperlink->uri);
		(*jniEnv)->DeleteGlobalRef(jniEnv, jaw_hyperlink->jstrUri);
	}

	jaw_hyperlink->jstrUri = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
	jaw_hyperlink->uri = (gchar*)(*jniEnv)->GetStringUTFChars( jniEnv, jaw_hyperlink->jstrUri, NULL );

	return jaw_hyperlink->uri;
}

static AtkObject*
jaw_hyperlink_get_object (AtkHyperlink *atk_hyperlink,
			gint i)
{
	JawHyperlink *jaw_hyperlink = JAW_HYPERLINK(atk_hyperlink);
	jobject jhyperlink = jaw_hyperlink->jhyperlink;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHyperlink = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHyperlink, "get_object", "(I)Ljava/lang/String;");
	jobject jobj = (*jniEnv)->CallObjectMethod(jniEnv, jhyperlink, jmid, (jint)i);
	if (jobj == NULL) {
		return NULL;
	}

	jclass classAccessible = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/Accessible" );
	jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessible, "getAccessibleContext", "()Ljavax/accessibility/AccessibleContext;" );
	jobject ac = (*jniEnv)->CallObjectMethod( jniEnv, jobj, jmid );

	AtkObject *obj = (AtkObject*) jaw_impl_get_instance( jniEnv, ac );

	return obj;
}

static gint
jaw_hyperlink_get_end_index (AtkHyperlink *atk_hyperlink)
{
	JawHyperlink *jaw_hyperlink = JAW_HYPERLINK(atk_hyperlink);
	jobject jhyperlink = jaw_hyperlink->jhyperlink;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHyperlink = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHyperlink, "get_end_index", "()I");
	jint jindex = (*jniEnv)->CallIntMethod(jniEnv, jhyperlink, jmid);

	return jindex;
}

static gint jaw_hyperlink_get_start_index (AtkHyperlink	*atk_hyperlink)
{
	JawHyperlink *jaw_hyperlink = JAW_HYPERLINK(atk_hyperlink);
	jobject jhyperlink = jaw_hyperlink->jhyperlink;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHyperlink = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHyperlink, "get_start_index", "()I");
	jint jindex = (*jniEnv)->CallIntMethod(jniEnv, jhyperlink, jmid);

	return jindex;
}

static gboolean jaw_hyperlink_is_valid (AtkHyperlink *atk_hyperlink)
{
	JawHyperlink *jaw_hyperlink = JAW_HYPERLINK(atk_hyperlink);
	jobject jhyperlink = jaw_hyperlink->jhyperlink;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHyperlink = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHyperlink, "is_valid", "()Z");
	jboolean jvalid = (*jniEnv)->CallBooleanMethod(jniEnv, jhyperlink, jmid);

	return jvalid;
}

static gint jaw_hyperlink_get_n_anchors (AtkHyperlink *atk_hyperlink)
{
	JawHyperlink *jaw_hyperlink = JAW_HYPERLINK(atk_hyperlink);
	jobject jhyperlink = jaw_hyperlink->jhyperlink;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHyperlink = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHyperlink, "get_n_anchors", "()I");
	jint janchors = (*jniEnv)->CallIntMethod(jniEnv, jhyperlink, jmid);

	return janchors;
}

