require "rake/extensiontask"
require 'rspec'
require 'rspec/core/rake_task'

Rake::ExtensionTask.new "opcua_client" do |ext|
  ext.lib_dir = "lib/opcua_client"
end

RSpec::Core::RakeTask.new('spec') do |t|
  t.verbose = true
end
