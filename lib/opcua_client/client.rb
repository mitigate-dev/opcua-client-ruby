module OPCUAClient
  class Client
    def configure_monitoring(items, callback)
      @items = items

      # TODO: monitor all items
      if items.first
        @mon_ns_index = items.first[0]
        @mon_ns_name = items.first[1]
      end

      @callback = callback
    end
  end
end
