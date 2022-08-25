require 'rspec'
require 'opcua_client'

RSpec.configure do |config|
  def new_client(connect: true)
    client = OPCUAClient::Client.new

    if connect
      # TODO
    end

    client
  end
end
