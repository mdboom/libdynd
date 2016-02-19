﻿Elementwise Reduction UFuncs
=============================

A number of useful reduction operations can be
computed in an elementwise fashion.

The particular functions we're most interested in are:

 * `all`, `any`: reduction on boolean `and`, `or`.
 * `sum`, `product`: reduction on `add`, `multiply`.
 * `max`, `min`: reduction on `maximum`, `minimum`.
 * `mean`, `std`, `var`: arithmetic mean, standard
   deviation, and variance.
 * `nansum`, `nanmin`, `nanmax`, `nanmean`, `nanstd`,
   `nanvar`: Versions of the above which map NaN values
   to "nonexistent", i.e. the array [1, NaN, 3] is
   considered to have length 2 instead of 3 like normal.

See also

* [Rolling Reduction UFuncs](rolling-reduction-ufuncs.md)

Keyword Arguments of Elementwise Reductions
-------------------------------------------

NumPy has two keyword arguments that are worth
keeping in this kind of operation for DyND,
`axis=` and `keepdims=`. The parameter to `axis`
may be a single integer, or, when the operation
is commutative and associative, a tuple of integers.
The `keepdims` parameter keeps the reduced dimensions
around as size 1 instead of removing them, so the
result still broadcasts appropriately against
the original.

Properties of Elementwise Reductions
-------------------------------------

### Input Type

This is the type of the elements that are being reduced.

### Accumulation Type

The accumulation type may be different from the
input type in a reduction. For example, for `mean`,
it may be a struct `{sum: float64, count: int64}`,
where the input type is `float64`.

### Initial State (Identity Element)

When the input and accumulation types are the same,
a reduction may have an identity, which is an element
`x` such that `op(x, y) == y` for all `y`. The identity
may be used as the initial state of the accumulators.

When the input and accumulation types are different, this
algebraic identity does not make sense, but an initial
state such as `[0.0, 0]` in the `mean` case still
makes sense.

### Initialization Function

When there is no initial state that can be used,
an initialization function which sets the initial
state from the first element being accumulated may
be used instead. Most commonly, this will be a copy
operation, but when the input and accumulation types
are different, something different may be required.
For NumPy compatibility, only the copy operation is
needed.

In NumPy, the NDIter object has the ability to
report whether a reduction operand is being
visited for the first time. This exists for the purpose
of knowing when to call the initialization function.
[See the NumPy Doc for this.](http://docs.scipy.org/doc/numpy/reference/c-api.iterator.html#NpyIter_IsFirstVisit)

## Accumulator Combine Function

When a reduction kernel is associative, it may be
useful to provide a function that combines accumulator
values together. For example, in the case of `mean`,
it would add the `sum` and `count` fields of the
inputs together. This may be used by a multi-threaded
execution engine.

### Associativity and Commutativity

An elementwise reduction might be associative
and/or commutative. Associativity means the identity
`op(op(x, y), z) == op(x, op(y, z))` holds for all
`x`, `y`, and `z`. Commutativity means the identity
`op(x, y) == op(y, x)` holds for all `x` and `y`.
When both of these hold, the order in which elements
are visited can be arbitrarily rearranged, which
is useful for optimizing memory access patterns and
exploiting concurrency.

This information needs to get to the execution
engine somehow, to affect how the elementwise
reduction ckernel is lifted to an assignment
ckernel, or how distributed scheduling is controlled.

Per-Kernel vs Per-Function Properties
-------------------------------------

The identity and the associativity/commutativity
flags must be per-kernel properties. To see this,
we'll give a few examples.

First, consider `product`. For integers, floats,
complex, and quaternions, it has identity `1`.
For all but quaternions, it is both associative and
commutative. This means `nd.product(complex_2d_array)`
makes sense, but `nd.product(quat_2d_array)` does
not, because the lack of commutativity prevents an
unambiguous definition.

Similarly, consider `sum`. For numeric types, it's
both associative and commutative, but for strings,
it's not commutative. This results in the same situation
as `product` does for quaternions.

```
>>> nd.sum([[1, 2], [3, 4]])
array(10,
      dshape='int32')

>>> nd.sum([[1, 2], [3, 4]], axis=0)
array([4, 6],
      dshape='2 * int32')

>>> nd.sum([[1, 2], [3, 4]], axis=1)
array([3, 7],
      dshape='2 * int32')

>>> nd.sum([['this', 'is'], ['a', 'test']])
DyNDError: nd.sum output of 2D reduction is ambiguous
due to lack of commutativity

>>> nd.sum([['this', 'is'], ['a', 'test']], axis=0)
array(['thisa', 'istest'],
      dshape='2 * string')

>>> nd.sum([['this', 'is'], ['a', 'test']], axis=1)
array(['thisis', 'atest'],
      dshape='2 * string')

>>> nd.max([['this', 'is'], ['a', 'test']])
array('this',
      dshape='string')
```

Implementation Details
----------------------

### Reduction CKernel

A reduction ckernel uses the same prototype as
an assignment ckernel, except it both reads and writes
from the destination memory. When it is called,
the destination stride may be 0 (dot-product style)
or non-zero (saxpy style). For our `mean` function
example, we might code it like (pseudo-code like):

```
void kfunc(char *dst, intptr_t dst_stride,
           const char *src, intptr_t src_stride,
			 size_t count,
           ckernel_prefix *extra)
{
	if (dst_stride == 0) {
       // dot-product style
       dst->sum += sum(src, src_stride, count);
       dst->count += count;
   } else {
       // saxpy style
       for (i = 0; i < count; ++i) {
           dst->sum += *src;
           dst->count ++;
           dst += dst_stride;
           src += src_stride;
       }
   }
}
```

### Lifting a Reduction CKernel

Lifting a reduction ckernel means adding code which
handles the dimensions appropriately, matching the
output appropriately depending on whether the
`keepdims` argument was provided as True, and
processing dimensions differently based on which
values are in the `axis` argument.

In the more general case, we want to also be able
to lift a "map", i.e. a ckernel which takes any
number of inputs, and broadcasts them elementwise
through an expression, followed by a "reduce". For
the loops to be fused properly in the computation,
this combined "map-reduce" must be lifted as one
entity.

In the case of `mean`, we then want to tack on another
"map" which takes the sum divided by the count, but
that can be separate from the reduction lifting.

Reduction ArrFunc signatures
----------------------------

Reduction function signatures operate via a dependent
type mechanism. The way this works is a reduction
function has a type signature like

```
(DimsIn... * InputDType) -> DimsOut... * OutputDType
```

and a Python function has an opportunity to fill
in the type variable values in the return type. The
reduction arrfunc accepts ``axis=`` and ``keepdims=``
keyword arguments, which are used to control how
the dependent return type is computed. Here's
some example code:

```
>>> a = nd.array(..., dshape='3 * 5 * var * int32')
>>> b = sum(a, axis=1, keepdims=False)
>>> b.dshape
dshape('3 * 1 * var * int32')
```

In this example, the ``DimsIn...`` type variable matched
with ``3 * 5 * var *``, and ``InputDType`` matched with
``int32``. For ``sum``, the input and output dtypes are
the same, and the parameters ``axis=1, keepdims=False``
give the value ``3 * 1 * var *`` to ``DimsOut...``.

Here are some examples of what the type signatures
of various reductions look like for a `sum` example.

```
>>> a
array([...],
      dshape='3 * var * 5 * int32')
>>> nd.sum(a).sig
'(3 * var * 5 * int32) -> int32'

>>> nd.sum(a, keepdims=True).sig
'(3 * var * 5 * int32) -> 1 * 1 * 1 * int32'

>>> nd.sum(a, axis=0).sig
'(3 * var * 5 * int32) -> var * 5 * int32'

>>> nd.sum(a, axis=1).sig
'(3 * var * 5 * int32) -> 3 * 5 * int32'

>>> nd.sum(a, axis=[0,2], keepdims=True).sig
'(3 * var * 5 * int32) -> 1 * var * 1 * int32'

>>> nd.sum(a, axis=[0,2]).sig
'(3 * var * 5 * int32) -> var * int32'

>>> nd.sum(a, axis=[1,2], keepdims=True).sig
'(3 * var * 5 * int32) -> 3 * 1 * 1 * int32'

>>> nd.sum(a, axis=[1,2]).sig
'(3 * var * 5 * int32) -> 3 * int32'
```

Something not fully addressed yet is in the case of
`mean`, `std`, and `var`, where there needs to be a way
to tack on the final divide or expression that computes
the final value after the reduction accumulators
are finished.
