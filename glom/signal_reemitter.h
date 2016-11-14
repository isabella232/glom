/* Glom
 *
 * Copyright (C) 2008 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_SIGNAL_REEMITTER_H
#define GLOM_SIGNAL_REEMITTER_H

#include <sigc++/sigc++.h>

namespace Glom
{

template<class T_sig_to_emit>
void reemit_0args(const T_sig_to_emit& sig_to_emit)
{
  sig_to_emit.emit();
}

template<class T_sig_to_emit, typename T_arg1>
void reemit_1arg(T_arg1 arg1, const T_sig_to_emit& sig_to_emit)
{
  sig_to_emit.emit(arg1);
}

template<class T_sig_to_emit, typename T_arg1, typename T_arg2>
void reemit_2args(T_arg1 arg1, T_arg2 arg2, const T_sig_to_emit& sig_to_emit)
{
  sig_to_emit.emit(arg1, arg2);
}

//Note that sig_to_catch is by-value instead of const-reference,
//because connect() is a non-const method,
//and a non-const-reference could not be used with a temporary instance.

/** Emit a signal when another signal is emitted.
 * @param sig_to_catch The signal to handle.
 * @param sig_to_emit The signal to emit when @a sig_to_catch is handled.
 */
template<class T_sig_to_catch, class T_sig_to_emit>
void signal_connect_for_reemit_0args(T_sig_to_catch sig_to_catch, const T_sig_to_emit& sig_to_emit)
{
  sig_to_catch.connect( sigc::bind( sigc::ptr_fun(&reemit_0args<T_sig_to_emit>), sig_to_emit) );
}

/** Emit a signal when another signal is emitted.
 * @param sig_to_catch The signal to handle.
 * @param sig_to_emit The signal to emit when @a sig_to_catch is handled.
 */
template<class T_arg1, class T_sig_to_emit>
void signal_connect_for_reemit_1arg(sigc::signal1<void, T_arg1> sig_to_catch, const T_sig_to_emit& sig_to_emit)
{
  sig_to_catch.connect( sigc::bind( sigc::ptr_fun(&reemit_1arg<T_sig_to_emit, T_arg1>), sig_to_emit) );
}

/** Emit a signal when another signal is emitted.
 * @param sig_to_catch The signal to handle.
 * @param sig_to_emit The signal to emit when @a sig_to_catch is handled.
 */
template<class T_arg1, class T_arg2, class T_sig_to_emit>
void signal_connect_for_reemit_2args(sigc::signal2<void, T_arg1, T_arg2> sig_to_catch, const T_sig_to_emit& sig_to_emit)
{
  sig_to_catch.connect( sigc::bind( sigc::ptr_fun(&reemit_2args<T_sig_to_emit, T_arg1, T_arg2>), sig_to_emit) );
}


} //namespace Glom

#endif //GLOM_SIGNAL_REEMITTER_H
