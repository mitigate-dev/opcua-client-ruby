source 'https://rubygems.org'

gemspec

if Gem::Version.new(RUBY_VERSION) >= Gem::Version.new("2.2")
  gem 'rake', '~> 13.0.1'
else
  gem 'rake', '< 13'
end

gem "rake-compiler"

group :development do
  gem 'pry'
end

group :test do
  gem 'rspec', '~> 3.11'
end
