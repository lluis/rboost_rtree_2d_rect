# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'rboost_rtree_2d_rect/version'

Gem::Specification.new do |spec|
  spec.name          = "rboost_rtree_2d_rect"
  spec.version       = RboostRtree2dRect::VERSION
  spec.authors       = ["Lluis Gili"]
  spec.email         = ["lluis@ingent.net"]
  spec.extensions    = ["ext/rboost_rtree_2d_rect/extconf.rb"]
  spec.summary       = %q{Ruby bindings for R-tree of BOOST library.}
  spec.description   = %q{Currently support only two dimensional space and rectangular objects.}
  spec.homepage      = "http://www.ssalewski.de/RT.html.en"
  spec.license       = "GPL-2.0"

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.6"
  spec.add_development_dependency "rake"
  spec.add_development_dependency "rake-compiler"
end
