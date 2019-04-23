require File.expand_path('../lib/opcua_client/version', __FILE__)

Gem::Specification.new do |s|
  s.name = 'opcua_client'
  s.version = OPCUAClient::VERSION
  s.authors = ['Ritvars Rundzans']
  s.license = "MIT"
  s.email = ['ritvars.rundzans@makit.lv']
  s.extensions = ["ext/opcua_client/extconf.rb"]
  s.homepage = 'https://github.com/mak-it/opcua-client-ruby'
  s.rdoc_options = ["--charset=UTF-8"]
  s.summary = 'Basic OPC-UA client library for Ruby. Uses open62541 (https://open62541.org) under the hood.'
  s.required_ruby_version = '>= 2.4.0'

  s.files = `git ls-files README.md CHANGELOG.md LICENSE ext lib support`.split
  s.test_files = `git ls-files spec examples`.split
end
