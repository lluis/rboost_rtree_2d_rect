# RboostRtree2dRect

Code taken from http://www.ssalewski.de/RT.html.en (author Stefan Salewski)

## Installation

To compile you need Boost Geometry development files.

*on Debian:*

    apt-get install -t jessie libboost-dev # won't compile with Wheezy version

or download source code from [sourceforge](http://sourceforge.net/projects/boost/files/boost/1.55.0/)


Add this line to your application's Gemfile:

    gem 'rboost_rtree_2d_rect'

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install rboost_rtree_2d_rect

## Usage

    require 'rboost_rtree_2d_rect/rboost_rtree_2d_rect'
    
    rt = BOOST::R_tree_2d_rect.new
    
    puts (rt.methods - Object.methods).sort
    
    =begin
    []
    []=
    delete
    each_object
    each_pair
    insert
    intersects?
    intersects_each?
    intersects_rect?
    intersects_rect_each?
    nearest
    nearest_each
    nearest_rect
    nearest_rect_each
    rect
    remove
    to_a
    update_or_insert
    =end
    
    rt.insert('r2277', 2, 2, 7, 7)
    p rt.rect('r2277') # query coorinates
    rt.insert('r3589', 3, 5, 8, 9)
    rt.insert('r1234', 1, 2, 3, 4)
    rt.insert('r3456', 1, 1, 9, 9) # wrong
    rt.update_or_insert('r3456', 2, 2, 8, 8) # still wrong
    rt['r3456'] = [3, 4, 5, 6] # correct
    
    p rt['r3589'] # [3, 5, 8, 9]
    
    puts rt.intersects?(0.9, 0.9, 2.1, 2.1) # should be r2277 and r1234
    
    puts rt.nearest(10, 10, 2) # should be r3589 and r2277
    
    rt.remove('r3589')
    puts rt.nearest(10, 10, 2) # now should be r3456 and r2277
    
    puts rt.nearest_rect(10, 10, 2) # returns array -- object and minx, miny, maxx, maxy
    
    rt.nearest_rect_each(10, 10, 2){|r| print r[0], ' has area ', (r[3] - r[1]) * (r[4] - r[2]), "\n"}
    
    rt.each_pair{|el| puts el}
    rt.delete('r2277')
    rt.each_pair{|el| puts el}
    
    a = rt.to_a
    a.each{|el| p el}

## Contributing

1. Fork it ( https://github.com/lluis/rboost_rtree_2d_rect/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request
