/*
 * gnome-keyring
 *
 * Copyright (C) 2011 Collabora Ltd.
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *  
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 *
 * Author: Stef Walter <stefw@collabora.co.uk>
 */

#ifndef EGG_HKDF_H_
#define EGG_HKDF_H_

#include <glib.h>

gboolean   egg_hkdf_perform                                   (const gchar *hash_algo,
                                                               gconstpointer input,
                                                               gsize n_input,
                                                               gconstpointer salt,
                                                               gsize n_salt,
                                                               gconstpointer info,
                                                               gsize n_info,
                                                               gpointer output,
                                                               gsize n_output);

#endif /* EGG_HKDF_H_ */
