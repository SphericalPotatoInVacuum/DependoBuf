package dbuf

// This file is autogenerated. Please, do not change it manually.

class Kek(val i: Long) {
    var j: Long = 0L

    @Throws(IllegalStateException::class) constructor(i: Long, j: Long) : this(i){
        this.j = j

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Kek) return false

        if (this.i != other.i) return false

        if (this.j != other.j) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Kek@${hashCode().toString(radix=16)}"
        return "Kek <i = $i> {j: $j}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Kek) return false

        if (this.j != other.j) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Kek {
            var return_object = Kek(0L)
            return return_object
        }
        fun make(j: Long): Kek {
            var return_object = Kek.default()
            return_object.j = j
            return return_object
        }
    }
}

class Constructor1(val n: Long, val m: Long, val k: Kek) {
    @Throws(IllegalStateException::class) constructor(n: Long, m: Long, k: Kek, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(n, m, k){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Constructor1) return false

        if (this.n != other.n) return false
        if (this.m != other.m) return false
        if (this.k != other.k) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Constructor1@${hashCode().toString(radix=16)}"
        return "Constructor1 <n = $n, m = $m, k = ${k.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Constructor1) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Constructor1 {
            var return_object = Constructor1(0L, 0L, Kek.default())
            return return_object
        }
        fun make(): Triple {
            var return_object = Triple.default()
            var inside_object = Constructor1.default()
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Constructor2(val n: Long, val m: Long, val k: Kek) {
    var x: Long = 0L

    @Throws(IllegalStateException::class) constructor(n: Long, m: Long, k: Kek, x: Long) : this(n, m, k){
        this.x = x

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Constructor2) return false

        if (this.n != other.n) return false
        if (this.m != other.m) return false
        if (this.k != other.k) return false

        if (this.x != other.x) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Constructor2@${hashCode().toString(radix=16)}"
        return "Constructor2 <n = $n, m = $m, k = ${k.toString(depth-1U)}> {x: $x}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Constructor2) return false

        if (this.x != other.x) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Constructor2 {
            var return_object = Constructor2(0L, 0L, Kek.default())
            return return_object
        }
        fun make(x: Long): Triple {
            var return_object = Triple.default()
            var inside_object = Constructor2.default()
            inside_object.x = x
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Constructor3(val n: Long, val m: Long, val k: Kek) {
    var y: Long = 0L
    lateinit var k1: Kek
    lateinit var k2: Kek

    @Throws(IllegalStateException::class) constructor(n: Long, m: Long, k: Kek, y: Long, k1: Kek, k2: Kek) : this(n, m, k){
        this.y = y
        this.k1 = k1
        this.k2 = k2

        check()
    }

    fun check() {
        check(this::k1.isInitialized) {"property k1 should be initialized"}
        check(this::k2.isInitialized) {"property k2 should be initialized"}

        check(k1.i == (y + n)) {"dependency i of k1 (is ${k1.i}) should be ${(y + n)}"}
        check(k2.i == ((k.j + k1.j) + m)) {"dependency i of k2 (is ${k2.i}) should be ${((k.j + k1.j) + m)}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Constructor3) return false

        if (this.n != other.n) return false
        if (this.m != other.m) return false
        if (this.k != other.k) return false

        if (this.y != other.y) return false
        if (this.k1 != other.k1) return false
        if (this.k2 != other.k2) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Constructor3@${hashCode().toString(radix=16)}"
        return "Constructor3 <n = $n, m = $m, k = ${k.toString(depth-1U)}> {y: $y, k1: ${k1.toString(depth-1U)}, k2: ${k2.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Constructor3) return false

        if (this.y != other.y) return false
        if (this.k1 notSameFields other.k1) return false
        if (this.k2 notSameFields other.k2) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Constructor3 {
            var return_object = Constructor3(0L, 0L, Kek.default())
            return return_object
        }
        fun make(y: Long, k1: Kek, k2: Kek): Triple {
            var return_object = Triple.default()
            var inside_object = Constructor3.default()
            inside_object.y = y
            inside_object.k1 = k1
            inside_object.k2 = k2
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Constructor4(val n: Long, val m: Long, val k: Kek) {
    @Throws(IllegalStateException::class) constructor(n: Long, m: Long, k: Kek, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(n, m, k){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Constructor4) return false

        if (this.n != other.n) return false
        if (this.m != other.m) return false
        if (this.k != other.k) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Constructor4@${hashCode().toString(radix=16)}"
        return "Constructor4 <n = $n, m = $m, k = ${k.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Constructor4) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Constructor4 {
            var return_object = Constructor4(0L, 0L, Kek.default())
            return return_object
        }
        fun make(): Triple {
            var return_object = Triple.default()
            var inside_object = Constructor4.default()
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Constructor5(val n: Long, val m: Long, val k: Kek) {
    @Throws(IllegalStateException::class) constructor(n: Long, m: Long, k: Kek, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(n, m, k){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Constructor5) return false

        if (this.n != other.n) return false
        if (this.m != other.m) return false
        if (this.k != other.k) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Constructor5@${hashCode().toString(radix=16)}"
        return "Constructor5 <n = $n, m = $m, k = ${k.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Constructor5) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Constructor5 {
            var return_object = Constructor5(0L, 0L, Kek.default())
            return return_object
        }
        fun make(): Triple {
            var return_object = Triple.default()
            var inside_object = Constructor5.default()
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Constructor7(val n: Long, val m: Long, val k: Kek) {
    @Throws(IllegalStateException::class) constructor(n: Long, m: Long, k: Kek, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(n, m, k){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Constructor7) return false

        if (this.n != other.n) return false
        if (this.m != other.m) return false
        if (this.k != other.k) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Constructor7@${hashCode().toString(radix=16)}"
        return "Constructor7 <n = $n, m = $m, k = ${k.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Constructor7) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Constructor7 {
            var return_object = Constructor7(0L, 0L, Kek.default())
            return return_object
        }
        fun make(): Triple {
            var return_object = Triple.default()
            var inside_object = Constructor7.default()
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Constructor8(val n: Long, val m: Long, val k: Kek) {
    @Throws(IllegalStateException::class) constructor(n: Long, m: Long, k: Kek, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(n, m, k){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Constructor8) return false

        if (this.n != other.n) return false
        if (this.m != other.m) return false
        if (this.k != other.k) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Constructor8@${hashCode().toString(radix=16)}"
        return "Constructor8 <n = $n, m = $m, k = ${k.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Constructor8) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Constructor8 {
            var return_object = Constructor8(0L, 0L, Kek.default())
            return return_object
        }
        fun make(): Triple {
            var return_object = Triple.default()
            var inside_object = Constructor8.default()
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Triple(val n: Long, val m: Long, val k: Kek) {
    lateinit var inside: Any

    @Throws(IllegalStateException::class) constructor(n: Long, m: Long, k: Kek, inside: Any) : this(n, m, k) {
        this.inside = inside
        check()
    }

    fun check() {
        check(k.i == (m + (m * m))) {"dependency i of k (is ${k.i}) should be ${(m + (m * m))}"}

        check(this::inside.isInitialized) {"property inside should be initialized"}

        if (n == 1L) {
            if (inside is Constructor1) (inside as Constructor1).check()
            else check(false) {"not valid inside"}
            return
        }
        if (n == 2L && m == 2L) {
            if (inside is Constructor2) (inside as Constructor2).check()
            else if (inside is Constructor3) (inside as Constructor3).check()
            else check(false) {"not valid inside"}
            return
        }
        if (m == 3L) {
            if (inside is Constructor4) (inside as Constructor4).check()
            else if (inside is Constructor5) (inside as Constructor5).check()
            else check(false) {"not valid inside"}
            return
        }
        if (n == 4L && m == -4L) {
            check(false) {"not valid inside"}
            return
        }
        if (true) {
            if (inside is Constructor7) (inside as Constructor7).check()
            else check(false) {"not valid inside"}
            return
        }
        if (n == 4L) {
            if (inside is Constructor8) (inside as Constructor8).check()
            else check(false) {"not valid inside"}
            return
        }
        check(false) {"not valid inside"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Triple) return false

        if (inside is Constructor1) return (inside as Constructor1).equals(other.inside)
        if (inside is Constructor2) return (inside as Constructor2).equals(other.inside)
        if (inside is Constructor3) return (inside as Constructor3).equals(other.inside)
        if (inside is Constructor4) return (inside as Constructor4).equals(other.inside)
        if (inside is Constructor5) return (inside as Constructor5).equals(other.inside)
        if (inside is Constructor7) return (inside as Constructor7).equals(other.inside)
        if (inside is Constructor8) return (inside as Constructor8).equals(other.inside)

        check(false) {"not valid inside"}
        return false
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Triple@${hashCode().toString(radix=16)}"

        if (inside is Constructor1) return "(Triple) ${(inside as Constructor1).toString(depth)}"
        if (inside is Constructor2) return "(Triple) ${(inside as Constructor2).toString(depth)}"
        if (inside is Constructor3) return "(Triple) ${(inside as Constructor3).toString(depth)}"
        if (inside is Constructor4) return "(Triple) ${(inside as Constructor4).toString(depth)}"
        if (inside is Constructor5) return "(Triple) ${(inside as Constructor5).toString(depth)}"
        if (inside is Constructor7) return "(Triple) ${(inside as Constructor7).toString(depth)}"
        if (inside is Constructor8) return "(Triple) ${(inside as Constructor8).toString(depth)}"

        check(false) {"not valid inside"}
        return "(Triple) Unknow"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Triple) return false

        if (inside is Constructor1) return (inside as Constructor1).sameFields(other.inside)
        if (inside is Constructor2) return (inside as Constructor2).sameFields(other.inside)
        if (inside is Constructor3) return (inside as Constructor3).sameFields(other.inside)
        if (inside is Constructor4) return (inside as Constructor4).sameFields(other.inside)
        if (inside is Constructor5) return (inside as Constructor5).sameFields(other.inside)
        if (inside is Constructor7) return (inside as Constructor7).sameFields(other.inside)
        if (inside is Constructor8) return (inside as Constructor8).sameFields(other.inside)

        check(false) {"not valid inside"}
        return false
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Triple {
            var return_object = Triple(0L, 0L, Kek.default())
            return return_object
        }
    }
}

