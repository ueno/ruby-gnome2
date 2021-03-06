# Copyright (C) 2015  Ruby-GNOME2 Project Team
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
  class Widget
    alias_method :events_raw, :events
    def events
      Gdk::EventMask.new(events_raw)
    end

    alias_method :add_events_raw, :add_events
    def add_events(new_events)
      unless new_events.is_a?(Gdk::EventMask)
        new_events = Gdk::EventMask.new(new_events)
      end
      add_events_raw(new_events.to_i)
    end

    alias_method :set_events_raw, :set_events
    def set_events(new_events)
      unless new_events.is_a?(Gdk::EventMask)
        new_events = Gdk::EventMask.new(new_events)
      end
      set_events_raw(new_events.to_i)
    end

    alias_method :events_raw=, :events=
    alias_method :events=, :set_events

    alias_method :drag_source_set_raw, :drag_source_set
    def drag_source_set(flags, targets, actions)
      targets = ensure_drag_targets(targets)
      drag_source_set_raw(flags, targets, actions)
    end

    alias_method :drag_dest_set_raw, :drag_dest_set
    def drag_dest_set(flags, targets, actions)
      targets = ensure_drag_targets(targets)
      drag_dest_set_raw(flags, targets, actions)
    end

    private
    def ensure_drag_targets(targets)
      return targets unless targets.is_a?(Array)

      targets.collect do |target|
        case target
        when Array
          TargetEntry.new(*target)
        else
          target
        end
      end
    end
  end
end
