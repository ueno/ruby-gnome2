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
  class TreeIter
    def model
      @model
    end

    def model=(model)
      @model = model
    end

    def get_value(column)
      @model.get_value(self, column).value
    end
    alias_method :[], :get_value

    def set_value(column, value)
      @model.set_value(self, column, value)
    end
    alias_method :[]=, :set_value

    def path
      @model.get_path(self)
    end
  end
end
