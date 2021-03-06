require "glib2"
require "gdk_pixbuf2"
require "cairo"

base_dir = Pathname.new(__FILE__).dirname.dirname.expand_path
vendor_dir = base_dir + "vendor" + "local"
vendor_bin_dir = vendor_dir + "bin"
GLib.prepend_dll_path(vendor_bin_dir)
begin
  major, minor, = RUBY_VERSION.split(/\./)
  require "#{major}.#{minor}/rsvg2.so"
rescue LoadError
  require "rsvg2.so"
end

module RSVG
  LOG_DOMAIN = "librsvg"

  class << self
    def cairo_available?
      true
    end
  end

  class Handle
    class << self
      # For backward compatibility
      def new_from_data(data)
        new(:data => data)
      end

      # For backward compatibility
      def new_from_file(file_name, options={})
        new(options.merge(:file_name => file_name))
      end
    end
  end
end

module Cairo
  class Context
    def render_rsvg_handle(handle, *args, &block)
      handle.render_cairo(self, *args, &block)
    end
  end
end

GLib::Log.set_log_domain(RSVG::LOG_DOMAIN)
