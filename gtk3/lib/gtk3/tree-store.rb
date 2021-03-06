# Copyright (C) 2014  Ruby-GNOME2 Project Team
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

module Gtk
  class TreeStore
    alias_method :initialize_raw, :initialize
    def initialize(*column_types)
      initialize_raw(column_types)
    end

    alias_method :insert_raw, :insert
    def insert(parent, position)
      iter = insert_raw(parent, position)
      iter.model = self
      iter
    end

    alias_method :insert_before_raw, :insert_before
    def insert_before(parent, sibling)
      iter = insert_before_raw(parent, sibling)
      iter.model = self
      iter
    end

    alias_method :insert_after_raw, :insert_after
    def insert_after(parent, sibling)
      iter = insert_after_raw(parent, sibling)
      iter.model = self
      iter
    end

    alias_method :prepend_raw, :prepend
    def prepend(parent)
      iter = prepend_raw(parent)
      iter.model = self
      iter
    end

    alias_method :append_raw, :append
    def append(parent)
      iter = append_raw(parent)
      iter.model = self
      iter
    end
  end
end
