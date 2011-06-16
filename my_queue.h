// Queue implementation -*- C++ -*-

// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
// Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

/** @file stl_queue.h
 *  This is an internal header file, included by other library headers.
 *  You should not attempt to use it directly.
 */

#ifndef _MY_QUEUE_H
#define _MY_QUEUE_H 1

using namespace std;

#include <deque>
#include <vector>
#include <bits/stl_heap.h>
#include <bits/stl_function.h>

  /**
   *  @brief  A standard container automatically sorting its contents.
   *
   *  @ingroup sequences
   *
   *  This is not a true container, but an @e adaptor.  It holds
   *  another container, and provides a wrapper interface to that
   *  container.  The wrapper is what enforces priority-based sorting 
   *  and %queue behavior.  Very few of the standard container/sequence
   *  interface requirements are met (e.g., iterators).
   *
   *  The second template parameter defines the type of the underlying
   *  sequence/container.  It defaults to std::vector, but it can be
   *  any type that supports @c front(), @c push_back, @c pop_back,
   *  and random-access iterators, such as std::deque or an
   *  appropriate user-defined type.
   *
   *  The third template parameter supplies the means of making
   *  priority comparisons.  It defaults to @c less<value_type> but
   *  can be anything defining a strict weak ordering.
   *
   *  Members not found in "normal" containers are @c container_type,
   *  which is a typedef for the second Sequence parameter, and @c
   *  push, @c pop, and @c top, which are standard %queue operations.
   *
   *  @note No equality/comparison operators are provided for
   *  %priority_queue.
   *
   *  @note Sorting of the elements takes place as they are added to,
   *  and removed from, the %priority_queue using the
   *  %priority_queue's member functions.  If you access the elements
   *  by other means, and change their data such that the sorting
   *  order would be different, the %priority_queue will not re-sort
   *  the elements for you.  (How could it know to do so?)
  */
  template<typename _Tp, typename _Sequence = vector<_Tp>,
	   typename _Compare  = less<typename _Sequence::value_type> >
    class peekable_priority_queue
  {
      // concept requirements
      typedef typename _Sequence::value_type _Sequence_value_type;
      __glibcxx_class_requires(_Tp, _SGIAssignableConcept)
      __glibcxx_class_requires(_Sequence, _SequenceConcept)
      __glibcxx_class_requires(_Sequence, _RandomAccessContainerConcept)
      __glibcxx_class_requires2(_Tp, _Sequence_value_type, _SameTypeConcept)
      __glibcxx_class_requires4(_Compare, bool, _Tp, _Tp,
				_BinaryFunctionConcept)

    public:
      typedef typename _Sequence::value_type                value_type;
      typedef typename _Sequence::reference                 reference;
      typedef typename _Sequence::const_reference           const_reference;
      typedef typename _Sequence::size_type                 size_type;
      typedef          _Sequence                            container_type;

    protected:
      //  See queue::c for notes on these names.
      _Sequence  c;
      _Compare   comp;

    public:
      /**
       *  @brief  Default constructor creates no elements.
       */
#ifndef __GXX_EXPERIMENTAL_CXX0X__
      explicit
      peekable_priority_queue(const _Compare& __x = _Compare(),
		     const _Sequence& __s = _Sequence())
      : c(__s), comp(__x)
      { std::make_heap(c.begin(), c.end(), comp); }
#else
      explicit
      peekable_priority_queue(const _Compare& __x,
		     const _Sequence& __s)
      : c(__s), comp(__x)
      { std::make_heap(c.begin(), c.end(), comp); }

      explicit
      peekable_priority_queue(const _Compare& __x = _Compare(),
		     _Sequence&& __s = _Sequence())
      : c(std::move(__s)), comp(__x)
      { std::make_heap(c.begin(), c.end(), comp); }
#endif

      /**
       *  @brief  Builds a %queue from a range.
       *  @param  first  An input iterator.
       *  @param  last  An input iterator.
       *  @param  x  A comparison functor describing a strict weak ordering.
       *  @param  s  An initial sequence with which to start.
       *
       *  Begins by copying @a s, inserting a copy of the elements
       *  from @a [first,last) into the copy of @a s, then ordering
       *  the copy according to @a x.
       *
       *  For more information on function objects, see the
       *  documentation on @link functors functor base
       *  classes@endlink.
       */
#ifndef __GXX_EXPERIMENTAL_CXX0X__
      template<typename _InputIterator>
        peekable_priority_queue(_InputIterator __first, _InputIterator __last,
		       const _Compare& __x = _Compare(),
		       const _Sequence& __s = _Sequence())
	: c(__s), comp(__x)
        {
	  __glibcxx_requires_valid_range(__first, __last);
	  c.insert(c.end(), __first, __last);
	  std::make_heap(c.begin(), c.end(), comp);
	}
#else
      template<typename _InputIterator>
        peekable_priority_queue(_InputIterator __first, _InputIterator __last,
		       const _Compare& __x,
		       const _Sequence& __s)
	: c(__s), comp(__x)
        {
	  __glibcxx_requires_valid_range(__first, __last);
	  c.insert(c.end(), __first, __last);
	  std::make_heap(c.begin(), c.end(), comp);
	}

      template<typename _InputIterator>
        peekable_priority_queue(_InputIterator __first, _InputIterator __last,
		       const _Compare& __x = _Compare(),
		       _Sequence&& __s = _Sequence())
	: c(std::move(__s)), comp(__x)
        {
	  __glibcxx_requires_valid_range(__first, __last);
	  c.insert(c.end(), __first, __last);
	  std::make_heap(c.begin(), c.end(), comp);
	}

      peekable_priority_queue(peekable_priority_queue&& __pq)
      : c(std::move(__pq.c)), comp(std::move(__pq.comp)) { }

      peekable_priority_queue&
      operator=(peekable_priority_queue&& __pq)
      {
	c = std::move(__pq.c);
	comp = std::move(__pq.comp);
	return *this;
      }
#endif

      /**
       *  Returns true if the %queue is empty.
       */
      bool
      empty() const
      { return c.empty(); }

      /**  Returns the number of elements in the %queue.  */
      size_type
      size() const
      { return c.size(); }

      /**
       *  Returns a read-only (constant) reference to the data at the first
       *  element of the %queue.
       */
      const_reference
      top() const
      {
	__glibcxx_requires_nonempty();
	return c.front();
      }

      /**
       *  @brief  Add data to the %queue.
       *  @param  x  Data to be added.
       *
       *  This is a typical %queue operation.
       *  The time complexity of the operation depends on the underlying
       *  sequence.
       */
      void
      push(const value_type& __x)
      {
	c.push_back(__x);
	std::push_heap(c.begin(), c.end(), comp);
      }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
      void
      push(value_type&& __x)
      {
	c.push_back(std::move(__x));
	std::push_heap(c.begin(), c.end(), comp);
      }

      template<typename... _Args>
        void
        emplace(_Args&&... __args)
	{
	  c.emplace_back(std::forward<_Args>(__args)...);
	  std::push_heap(c.begin(), c.end(), comp);
	}
#endif

      /**
       *  @brief  Removes first element.
       *
       *  This is a typical %queue operation.  It shrinks the %queue
       *  by one.  The time complexity of the operation depends on the
       *  underlying sequence.
       *
       *  Note that no data is returned, and if the first element's
       *  data is needed, it should be retrieved before pop() is
       *  called.
       */
      void
      pop()
      {
	__glibcxx_requires_nonempty();
	std::pop_heap(c.begin(), c.end(), comp);
	c.pop_back();
      }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
      void
      swap(peekable_priority_queue&& __pq)
      {
	using std::swap;
	c.swap(__pq.c);
	swap(comp, __pq.comp);
      }
#endif

  public:
    size_type size( void ) { return c.size(); }

    typedef typename container_type::const_iterator const_iterator;
    const_iterator begin( void ) { return c.begin(); }
    const_iterator end( void ) { return c.end(); }
  };

  // No equality/comparison operators are provided for peekable_priority_queue.

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  template<typename _Tp, typename _Sequence, typename _Compare>
    inline void
    swap(peekable_priority_queue<_Tp, _Sequence, _Compare>& __x,
	 peekable_priority_queue<_Tp, _Sequence, _Compare>& __y)
    { __x.swap(__y); }

  template<typename _Tp, typename _Sequence, typename _Compare>
    inline void
    swap(peekable_priority_queue<_Tp, _Sequence, _Compare>&& __x,
	 peekable_priority_queue<_Tp, _Sequence, _Compare>& __y)
    { __x.swap(__y); }

  template<typename _Tp, typename _Sequence, typename _Compare>
    inline void
    swap(peekable_priority_queue<_Tp, _Sequence, _Compare>& __x,
	 peekable_priority_queue<_Tp, _Sequence, _Compare>&& __y)
    { __x.swap(__y); }
#endif

#endif /* _STL_QUEUE_H */
