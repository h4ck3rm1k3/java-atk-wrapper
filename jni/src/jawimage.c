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

#include <atk/atk.h>
#include <glib.h>
#include "jawimpl.h"
#include "jawutil.h"

extern void	jaw_image_interface_init (AtkImageIface*);
extern gpointer	jaw_image_data_init (jobject ac);
extern void	jaw_image_data_finalize (gpointer);

static void			jaw_image_get_image_position		(AtkImage	*image,
									 gint		*x,
									 gint		*y,
									 AtkCoordType	coord_type);
static G_CONST_RETURN gchar*	jaw_image_get_image_description		(AtkImage	*image);
static void			jaw_image_get_image_size		(AtkImage	*image,
									 gint		*width,
									 gint		*height);

typedef struct _ImageData {
	jobject atk_image;
	gchar* image_description;
	jstring jstrImageDescription;
} ImageData;

void
jaw_image_interface_init (AtkImageIface *iface)
{
	iface->get_image_position = jaw_image_get_image_position;
	iface->get_image_description = jaw_image_get_image_description;
	iface->set_image_description = NULL;
	iface->get_image_size = jaw_image_get_image_size;
}

gpointer
jaw_image_data_init (jobject ac)
{
	ImageData *data = g_new0(ImageData, 1);

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classImage = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkImage");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classImage, "<init>", "(Ljavax/accessibility/AccessibleContext;)V");
	jobject jatk_image = (*jniEnv)->NewObject(jniEnv, classImage, jmid, ac);
	data->atk_image = (*jniEnv)->NewGlobalRef(jniEnv, jatk_image);

	return data;
}

void
jaw_image_data_finalize (gpointer p)
{
	ImageData *data = (ImageData*)p;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	if (data && data->atk_image) {
		if (data->image_description != NULL) {
			(*jniEnv)->ReleaseStringUTFChars(jniEnv, data->jstrImageDescription, data->image_description);
			(*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrImageDescription);
			data->jstrImageDescription = NULL;
			data->image_description = NULL;
		}


		(*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_image);
		data->atk_image = NULL;
	}
}

static void
jaw_image_get_image_position (AtkImage *image,
		gint *x, gint *y, AtkCoordType coord_type)
{
	JawObject *jaw_obj = JAW_OBJECT(image);
	ImageData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_IMAGE);
	jobject atk_image = data->atk_image;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkImage = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkImage");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkImage, "get_image_position", "(I)Ljava/awt/Point;");
	jobject jpoint = (*jniEnv)->CallObjectMethod(jniEnv, atk_image, jmid, (jint)coord_type);
	
	if (jpoint == NULL) {
		(*x) = 0;
		(*y) = 0;
		return;
	}

	jclass classPoint = (*jniEnv)->FindClass(jniEnv, "java/awt/Point");
	jfieldID jfidX = (*jniEnv)->GetFieldID(jniEnv, classPoint, "x", "I");
	jfieldID jfidY = (*jniEnv)->GetFieldID(jniEnv, classPoint, "y", "I");
	jint jx = (*jniEnv)->GetIntField(jniEnv, jpoint, jfidX);
	jint jy = (*jniEnv)->GetIntField(jniEnv, jpoint, jfidY);

	(*x) = (gint)jx;
	(*y) = (gint)jy;
}

static G_CONST_RETURN gchar*
jaw_image_get_image_description (AtkImage *image)
{
	JawObject *jaw_obj = JAW_OBJECT(image);
	ImageData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_IMAGE);
	jobject atk_image = data->atk_image;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkImage = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkImage");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkImage, "get_image_description", "()Ljava/lang/String;");
	jstring jstr = (*jniEnv)->CallObjectMethod(jniEnv, atk_image, jmid);
	
	if (data->image_description != NULL) {
		(*jniEnv)->ReleaseStringUTFChars(jniEnv, data->jstrImageDescription, data->image_description);
		(*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrImageDescription);
	}
	
	data->jstrImageDescription = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
	data->image_description = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv, data->jstrImageDescription, NULL);

	return data->image_description;
}

static void
jaw_image_get_image_size (AtkImage *image, gint *width, gint *height)
{
	JawObject *jaw_obj = JAW_OBJECT(image);
	ImageData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_IMAGE);
	jobject atk_image = data->atk_image;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkImage = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkImage");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkImage, "get_image_size", "()Ljava/awt/Dimension;");
	jobject jdimension = (*jniEnv)->CallObjectMethod(jniEnv, atk_image, jmid);

	if (jdimension == NULL) {
		(*width) = 0;
		(*height) = 0;
		return;
	}

	jclass classDimension = (*jniEnv)->FindClass(jniEnv, "java/awt/Dimension");
	jfieldID jfidWidth = (*jniEnv)->GetFieldID(jniEnv, classDimension, "width", "I");
	jfieldID jfidHeight = (*jniEnv)->GetFieldID(jniEnv, classDimension, "height", "I");
	jint jwidth = (*jniEnv)->GetIntField(jniEnv, jdimension, jfidWidth);
	jint jheight = (*jniEnv)->GetIntField(jniEnv, jdimension, jfidHeight);

	(*width) = (gint)jwidth;
	(*height) = (gint)jheight;
}

