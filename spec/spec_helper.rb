require 'rspec'
require 'opcua_client'

# https://github.com/brianmario/mysql2/commit/0ee20536501848a354f1c3a007333167120c7457
if GC.respond_to?(:verify_compaction_references)
  # This method was added in Ruby 3.0.0. Calling it this way asks the GC to
  # move objects around, helping to find object movement bugs.
  GC.verify_compaction_references(double_heap: true, toward: :empty)
end

def new_client(connect: true)
  client = OPCUAClient::Client.new

  if connect
    client.connect("opc.tcp://127.0.0.1:4840")
  end

  client
end

RSpec.configure do |config|
end
