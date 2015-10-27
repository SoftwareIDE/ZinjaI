#                                                                                                        
#   STL GDB evaluators/views/utilities                                                     
#
#	Estas macros estan basadas en el trabajo de Dan Marinescu, y modificadas
#   para una mejor interpretación por parte de ZinjaI.
#   El siguiente comentario en inglés forma parte del archivo original.
#
#   Simple GDB Macros writen by Dan Marinescu (H-PhD) - License GPL
#   Inspired by intial work of Tom Malnar, 
#     Tony Novac (PhD) / Cornell / Stanford,
#     Gilad Mishne (PhD) and Many Many Others.
#   Contact: dan_c_marinescu@yahoo.com (Subject: STL)
#

define pvector
	if $argc == 0
		help pvector
	else
		set $size = ($arg0)._M_impl._M_finish - ($arg0)._M_impl._M_start
		set $capacity = ($arg0)._M_impl._M_end_of_storage - ($arg0)._M_impl._M_start
		set $size_max = $size - 1
		printf "{"
	end
	if $argc == 1
		set $i = 0
		while $i < $size
			printf "%u=", $i
			p *(($arg0)._M_impl._M_start + $i)
			printf ", "
			set $i++
		end
	end
	if $argc == 2
		set $idx = $arg1
		if $idx < 0 || $idx > $size_max
			printf "Rango incorrecto: [0..%u].\n", $size_max
		else
			printf "%u=", $idx
			p *(($arg0)._M_impl._M_start + $idx)
			printf ", "
		end
	end
	if $argc == 3
	  set $start_idx = $arg1
	  set $stop_idx = $arg2
	  if $start_idx > $stop_idx
	    set $tmp_idx = $start_idx
	    set $start_idx = $stop_idx
	    set $stop_idx = $tmp_idx
	  end
	  if $start_idx < 0 || $stop_idx < 0 || $start_idx > $size_max || $stop_idx > $size_max
	    printf "Rango incorrecto: [0..%u].\n", $size_max
	  else
	    set $i = $start_idx
		while $i <= $stop_idx
			printf "%u=", $i
			p *(($arg0)._M_impl._M_start + $i)
			printf ", "
			set $i++
		end
	  end
	end
	if $argc > 0
		printf "size=%u, ", $size
		printf "capacity=%u, ", $capacity
		printf "Element="
		whatis *($arg0)._M_impl._M_start
		printf "}\n"
	end
end
document pvector
Use: pvector <vector> [<from> [<to>]]
end



define plist
	if $argc == 0
		help plist
	else
		printf "{"
		set $head = &($arg0)._M_impl._M_node
		set $current = ($arg0)->_M_impl->_M_node->_M_next
		set $size = 0
		while $current != $head
			if $argc == 2
				printf "%u=", $size
				p *($arg1*)($current + 1)
				printf ", "
			end
			if $argc == 3
				if $size == $arg2
					printf "%u=", $size
					p *($arg1*)($current + 1)
					printf ", "
				end
			end
			set $current = $current->_M_next
			set $size++
		end
		printf "size=%u", $size
		if $argc == 1
			help plist
		end
		printf "}\n"
	end
end
document plist
Use: plist <list> <type> [<index>]
end

define pmap
	if $argc == 0
		printf "Use \"pmap <objeto> <tipo1> <tipo2>\".\n"
	else
		set $tree = ($arg0)
		set $i = 0
		set $node = $tree._M_t._M_impl._M_header._M_left
		set $end = $tree._M_t._M_impl._M_header
		set $tree_size = $tree._M_t._M_impl._M_node_count
		printf "{"
		if $argc == 3
			while $i < $tree_size
				set $value = (void *)($node + 1)
				p *($arg1*)$value
				set $value = $value + 4
				printf "="
				p *($arg2*)$value
				printf ", "
				if $node->_M_right != 0
					set $node = $node->_M_right
					while $node->_M_left != 0
						set $node = $node->_M_left
					end
				else
					set $tmp_node = $node->_M_parent
					while $node == $tmp_node->_M_right
						set $node = $tmp_node
						set $tmp_node = $tmp_node->_M_parent
					end
					if $node->_M_right != $tmp_node
						set $node = $tmp_node
					end
				end
				set $i++
			end
		end
		if $argc == 4
			set $idx = $arg3
			set $ElementsFound = 0
			while $i < $tree_size
				set $value = (void *)($node + 1)
				if *($arg1*)$value == $idx
					p *($arg1*)$value
					set $value = $value + 4
					printf "="
					p *($arg2*)$value
					printf ", "
					set $ElementsFound++
				end
				if $node->_M_right != 0
					set $node = $node->_M_right
					while $node->_M_left != 0
						set $node = $node->_M_left
					end
				else
					set $tmp_node = $node->_M_parent
					while $node == $tmp_node->_M_right
						set $node = $tmp_node
						set $tmp_node = $tmp_node->_M_parent
					end
					if $node->_M_right != $tmp_node
						set $node = $tmp_node
					end
				end
				set $i++
			end
			printf "found=%u, ", $ElementsFound
		end
		if $argc == 5
			set $idx1 = $arg3
			set $idx2 = $arg4
			set $ElementsFound = 0
			while $i < $tree_size
				set $value = (void *)($node + 1)
				set $valueLeft = *($arg1*)$value
				set $valueRight = *($arg2*)($value + 4)
				if $valueLeft == $idx1 && $valueRight == $idx2
					p $valueLeft
					printf "="
					p $valueRight
					printf ", "
					set $ElementsFound++
				end
				
				if $node->_M_right != 0
					set $node = $node->_M_right
					while $node->_M_left != 0
						set $node = $node->_M_left
					end
				else
					set $tmp_node = $node->_M_parent
					while $node == $tmp_node->_M_right
						set $node = $tmp_node
						set $tmp_node = $tmp_node->_M_parent
					end
					if $node->_M_right != $tmp_node
						set $node = $tmp_node
					end
				end
				
				set $i++
			end
			printf "found=%u, ", $ElementsFound
		end
		printf "size=%u", $tree_size
		if $argc == 1 || $argc==2
			help pmap
		end
		printf "}\n"
	end
end
document pmap
Use: pmap <map> <type_key> <type_value>
end

define pstack
	if $argc == 0
		help pstack
	else
		printf "{"
		set $start_cur = ($arg0).c._M_impl._M_start._M_cur
		set $finish_cur = ($arg0).c._M_impl._M_finish._M_cur
		set $size = $finish_cur - $start_cur
        set $i = $size - 1
        while $i >= 0
			printf "%u=", $i
            p *($start_cur + $i)
			printf ", "
            set $i--
        end
		printf "size=%u}\n", $size
	end
end
document pstack
Use: pstack <stack>
end

define pset
	if $argc == 0
		help pset
	else
		set $tree = ($arg0)
		set $i = 0
		set $node = $tree._M_t._M_impl._M_header._M_left
		set $end = $tree._M_t._M_impl._M_header
		set $tree_size = $tree._M_t._M_impl._M_node_count
		printf "{"
		if $argc == 2
			while $i < $tree_size
				set $value = (void *)($node + 1)
				printf "%u=", $i
				p *($arg1*)$value
				printf ", "
				if $node->_M_right != 0
					set $node = $node->_M_right
					while $node->_M_left != 0
						set $node = $node->_M_left
					end
				else
					set $tmp_node = $node->_M_parent
					while $node == $tmp_node->_M_right
						set $node = $tmp_node
						set $tmp_node = $tmp_node->_M_parent
					end
					if $node->_M_right != $tmp_node
						set $node = $tmp_node
					end
				end
				set $i++
			end
		end
		if $argc == 3
			set $idx = $arg2
			set $ElementsFound = 0
			while $i < $tree_size
				set $value = (void *)($node + 1)
				if *($arg1*)$value == $idx
					printf "%u=", $i
					p *($arg1*)$value
					printf ", "
					set $ElementsFound++
				end
				if $node->_M_right != 0
					set $node = $node->_M_right
					while $node->_M_left != 0
						set $node = $node->_M_left
					end
				else
					set $tmp_node = $node->_M_parent
					while $node == $tmp_node->_M_right
						set $node = $tmp_node
						set $tmp_node = $tmp_node->_M_parent
					end
					if $node->_M_right != $tmp_node
						set $node = $tmp_node
					end
				end
				set $i++
			end
			printf "found=%u, ", $ElementsFound
		end
		printf "size=%u", $tree_size
		printf "}\n"
	end
end
document pset
Use: pset <set> [<type> [<element>]]
end

define pdeque
	if $argc == 0
		help pdeque
	else
		printf "{"
		set $size = 0
		set $start_cur = ($arg0)._M_impl._M_start._M_cur
		set $start_last = ($arg0)._M_impl._M_start._M_last
		set $start_stop = $start_last
		while $start_cur != $start_stop
			printf "%u=", $size
			p *$start_cur
			printf ", "
			set $start_cur++
			set $size++
		end
		set $finish_first = ($arg0)._M_impl._M_finish._M_first
		set $finish_cur = ($arg0)._M_impl._M_finish._M_cur
		set $finish_last = ($arg0)._M_impl._M_finish._M_last
		if $finish_cur < $finish_last
			set $finish_stop = $finish_cur
		else
			set $finish_stop = $finish_last
		end
		while $finish_first != $finish_stop
			printf "%u=", $size
			p *$finish_first
			printf ", "
			set $finish_first++
			set $size++
		end
		printf "size=%u}\n", $size
	end
end
document pdeque
Use: pdeque <deque>
end

define pqueue
	if $argc == 0
		help pqueue
	else
		printf "{"
		set $start_cur = ($arg0).c._M_impl._M_start._M_cur
		set $finish_cur = ($arg0).c._M_impl._M_finish._M_cur
		set $size = $finish_cur - $start_cur
        set $i = 0
        while $i < $size
            printf "%u=", $size
			p *($start_cur + $i)
			printf ", "
            set $i++
        end
		printf "size=%u}\n", $size
	end
end
document pqueue
Use: pqueue <queue>
end

define ppqueue
	if $argc == 0
		help ppqueue
	else
		printf "{"
		set $size = ($arg0).c._M_impl._M_finish - ($arg0).c._M_impl._M_start
		set $capacity = ($arg0).c._M_impl._M_end_of_storage - ($arg0).c._M_impl._M_start
		set $i = $size - 1
		while $i >= 0
            printf "%u=", $size
			p *(($arg0).c._M_impl._M_start + $i)
			printf ", "
			set $i--
		end
		printf "size=%u, ", $size
		printf "capacity=%u}\n", $capacity
	end
end
document pqueue
Use: ppqueue <priority_queue>
end


define pbitset
	if $argc == 0
		help pbitset
	else
        p /t ($arg0)._M_w
	end
end
document pbitset
Use: pbitset <bitset>
end

define pstring
	if $argc == 0
		help pstring
	else
		printf "{
		printf "string=\"%s\"", ($arg0)._M_data()
		printf ", size/length=%u", ($arg0)._M_rep()->_M_length
		printf ", capacity=%u", ($arg0)._M_rep()->_M_capacity
		printf ", ref-count=%d", ($arg0)._M_rep()->_M_refcount
		printf "}\n"
	end
end
document pstring
Use: pstring <string>
end

define pwstring
	if $argc == 0
		help pwstring
	else
		printf "{"
		call printf("string=\"%ls\"", ($arg0)._M_data())
		printf ", size/length=%u", ($arg0)._M_rep()->_M_length
		printf ", capacity=%u", ($arg0)._M_rep()->_M_capacity
		printf ", ref-count=%d", ($arg0)._M_rep()->_M_refcount
		printf "}\n"
	end
end
document pwstring
Use: pwstring <wstring>
end
