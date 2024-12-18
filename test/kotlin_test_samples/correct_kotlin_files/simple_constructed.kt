package dbuf

// This file is autogenerated. Please, do not change it manually.

class Constructable() {
    var f1: Long = 0L
    var f2: String = ""
    var f3: Boolean = false
    var f4: Long = 0L

    @Throws(IllegalStateException::class) constructor(f1: Long, f2: String, f3: Boolean, f4: Long) : this(){
        this.f1 = f1
        this.f2 = f2
        this.f3 = f3
        this.f4 = f4

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Constructable) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Constructable@${hashCode().toString(radix=16)}"
        return "Constructable {f1: $f1, f2: $f2, f3: $f3, f4: $f4}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Constructable) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Constructable {
            var return_object = Constructable()
            return return_object
        }
        fun make(f1: Long, f2: String, f3: Boolean, f4: Long): Constructable {
            var return_object = Constructable.default()
            return_object.f1 = f1
            return_object.f2 = f2
            return_object.f3 = f3
            return_object.f4 = f4
            return return_object
        }
    }
}

class ConstructableD(val c: Constructable) {
    @Throws(IllegalStateException::class) constructor(c: Constructable, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(c){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is ConstructableD) return false

        if (this.c != other.c) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.ConstructableD@${hashCode().toString(radix=16)}"
        return "ConstructableD <c = ${c.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is ConstructableD) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : ConstructableD {
            var return_object = ConstructableD(Constructable.default())
            return return_object
        }
        fun make(): ConstructableD {
            var return_object = ConstructableD.default()
            return return_object
        }
    }
}

class Simple() {
    var f1: Long = 0L

    @Throws(IllegalStateException::class) constructor(f1: Long) : this(){
        this.f1 = f1

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Simple) return false

        if (this.f1 != other.f1) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Simple@${hashCode().toString(radix=16)}"
        return "Simple {f1: $f1}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Simple) return false

        if (this.f1 != other.f1) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Simple {
            var return_object = Simple()
            return return_object
        }
        fun make(f1: Long): Simple {
            var return_object = Simple.default()
            return_object.f1 = f1
            return return_object
        }
    }
}

class Ctr1(val x: Long, val s: Simple) {
    var y: Long = 0L
    var z: Long = 0L

    @Throws(IllegalStateException::class) constructor(x: Long, s: Simple, y: Long, z: Long) : this(x, s){
        this.y = y
        this.z = z

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Ctr1) return false

        if (this.x != other.x) return false
        if (this.s != other.s) return false

        if (this.y != other.y) return false
        if (this.z != other.z) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Ctr1@${hashCode().toString(radix=16)}"
        return "Ctr1 <x = $x, s = ${s.toString(depth-1U)}> {y: $y, z: $z}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Ctr1) return false

        if (this.y != other.y) return false
        if (this.z != other.z) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Ctr1 {
            var return_object = Ctr1(0L, Simple.default())
            return return_object
        }
        fun make(y: Long, z: Long): EnumConstructable {
            var return_object = EnumConstructable.default()
            var inside_object = Ctr1.default()
            inside_object.y = y
            inside_object.z = z
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Ctr2(val x: Long, val s: Simple) {
    @Throws(IllegalStateException::class) constructor(x: Long, s: Simple, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(x, s){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Ctr2) return false

        if (this.x != other.x) return false
        if (this.s != other.s) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Ctr2@${hashCode().toString(radix=16)}"
        return "Ctr2 <x = $x, s = ${s.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Ctr2) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Ctr2 {
            var return_object = Ctr2(0L, Simple.default())
            return return_object
        }
        fun make(): EnumConstructable {
            var return_object = EnumConstructable.default()
            var inside_object = Ctr2.default()
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Ctr3(val x: Long, val s: Simple) {
    @Throws(IllegalStateException::class) constructor(x: Long, s: Simple, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(x, s){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Ctr3) return false

        if (this.x != other.x) return false
        if (this.s != other.s) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Ctr3@${hashCode().toString(radix=16)}"
        return "Ctr3 <x = $x, s = ${s.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Ctr3) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Ctr3 {
            var return_object = Ctr3(0L, Simple.default())
            return return_object
        }
        fun make(): EnumConstructable {
            var return_object = EnumConstructable.default()
            var inside_object = Ctr3.default()
            return_object.inside = inside_object
            return return_object
        }
    }
}

class EnumConstructable(val x: Long, val s: Simple) {
    lateinit var inside: Any

    @Throws(IllegalStateException::class) constructor(x: Long, s: Simple, inside: Any) : this(x, s) {
        this.inside = inside
        check()
    }

    fun check() {
        check(this::inside.isInitialized) {"property inside should be initialized"}

        if (s == Simple.make(f1 = 2L)) {
            if (inside is Ctr1) (inside as Ctr1).check()
            else check(false) {"not valid inside"}
            return
        }
        if (s == Simple.make(f1 = (x + x))) {
            if (inside is Ctr2) (inside as Ctr2).check()
            else check(false) {"not valid inside"}
            return
        }
        if (true) {
            if (inside is Ctr3) (inside as Ctr3).check()
            else check(false) {"not valid inside"}
            return
        }
        check(false) {"not valid inside"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is EnumConstructable) return false

        if (inside is Ctr1) return (inside as Ctr1).equals(other.inside)
        if (inside is Ctr2) return (inside as Ctr2).equals(other.inside)
        if (inside is Ctr3) return (inside as Ctr3).equals(other.inside)

        check(false) {"not valid inside"}
        return false
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.EnumConstructable@${hashCode().toString(radix=16)}"

        if (inside is Ctr1) return "(EnumConstructable) ${(inside as Ctr1).toString(depth)}"
        if (inside is Ctr2) return "(EnumConstructable) ${(inside as Ctr2).toString(depth)}"
        if (inside is Ctr3) return "(EnumConstructable) ${(inside as Ctr3).toString(depth)}"

        check(false) {"not valid inside"}
        return "(EnumConstructable) Unknow"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is EnumConstructable) return false

        if (inside is Ctr1) return (inside as Ctr1).sameFields(other.inside)
        if (inside is Ctr2) return (inside as Ctr2).sameFields(other.inside)
        if (inside is Ctr3) return (inside as Ctr3).sameFields(other.inside)

        check(false) {"not valid inside"}
        return false
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : EnumConstructable {
            var return_object = EnumConstructable(0L, Simple.default())
            return return_object
        }
    }
}

class MessageConstructorCheck(val v1: Long) {
    var f1: Boolean = false
    lateinit var c: ConstructableD

    @Throws(IllegalStateException::class) constructor(v1: Long, f1: Boolean, c: ConstructableD) : this(v1){
        this.f1 = f1
        this.c = c

        check()
    }

    fun check() {
        check(this::c.isInitialized) {"property c should be initialized"}

        check(c.c sameFields Constructable.make(f1 = v1, f2 = "i am second", f3 = f1, f4 = (1L - 2L))) {"dependency c of c (is ${c.c}) should be ${Constructable.make(f1 = v1, f2 = "i am second", f3 = f1, f4 = (1L - 2L))}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is MessageConstructorCheck) return false

        if (this.v1 != other.v1) return false

        if (this.f1 != other.f1) return false
        if (this.c != other.c) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.MessageConstructorCheck@${hashCode().toString(radix=16)}"
        return "MessageConstructorCheck <v1 = $v1> {f1: $f1, c: ${c.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is MessageConstructorCheck) return false

        if (this.f1 != other.f1) return false
        if (this.c notSameFields other.c) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : MessageConstructorCheck {
            var return_object = MessageConstructorCheck(0L)
            return return_object
        }
        fun make(f1: Boolean, c: ConstructableD): MessageConstructorCheck {
            var return_object = MessageConstructorCheck.default()
            return_object.f1 = f1
            return_object.c = c
            return return_object
        }
    }
}

