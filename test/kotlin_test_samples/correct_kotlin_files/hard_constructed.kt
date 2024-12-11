package dbuf

// This file is autogenerated. Please, do not change it manually.

class A(val i: Long) {
    var x: Long = 0L
    var y: String = ""

    @Throws(IllegalStateException::class) constructor(i: Long, x: Long, y: String) : this(i){
        this.x = x
        this.y = y

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is A) return false

        if (this.i != other.i) return false

        if (this.x != other.x) return false
        if (this.y != other.y) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.A@${hashCode().toString(radix=16)}"
        return "A <i = $i> {x: $x, y: $y}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is A) return false

        if (this.x != other.x) return false
        if (this.y != other.y) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : A {
            var return_object = A(0L)
            return return_object
        }
        fun make(x: Long, y: String): A {
            var return_object = A.default()
            return_object.x = x
            return_object.y = y
            return return_object
        }
    }
}

class B() {
    var x: Long = 0L
    lateinit var a: A

    @Throws(IllegalStateException::class) constructor(x: Long, a: A) : this(){
        this.x = x
        this.a = a

        check()
    }

    fun check() {
        check(this::a.isInitialized) {"property a should be initialized"}

        check(a.i == x) {"dependency i of a (is ${a.i}) should be ${x}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is B) return false

        if (this.x != other.x) return false
        if (this.a != other.a) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.B@${hashCode().toString(radix=16)}"
        return "B {x: $x, a: ${a.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is B) return false

        if (this.x != other.x) return false
        if (this.a notSameFields other.a) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : B {
            var return_object = B()
            return return_object
        }
        fun make(x: Long, a: A): B {
            var return_object = B.default()
            return_object.x = x
            return_object.a = a
            return return_object
        }
    }
}

class BD(val b: B) {
    @Throws(IllegalStateException::class) constructor(b: B, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(b){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is BD) return false

        if (this.b != other.b) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.BD@${hashCode().toString(radix=16)}"
        return "BD <b = ${b.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is BD) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : BD {
            var return_object = BD(B.default())
            return return_object
        }
        fun make(): BD {
            var return_object = BD.default()
            return return_object
        }
    }
}

class Cons2(val x: Long) {
    var f1: Long = 0L
    var f2: String = ""

    @Throws(IllegalStateException::class) constructor(x: Long, f1: Long, f2: String) : this(x){
        this.f1 = f1
        this.f2 = f2

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Cons2) return false

        if (this.x != other.x) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Cons2@${hashCode().toString(radix=16)}"
        return "Cons2 <x = $x> {f1: $f1, f2: $f2}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Cons2) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Cons2 {
            var return_object = Cons2(0L)
            return return_object
        }
        fun make(f1: Long, f2: String): HardEnum {
            var return_object = HardEnum.default()
            var inside_object = Cons2.default()
            inside_object.f1 = f1
            inside_object.f2 = f2
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Cons3(val x: Long) {
    var f3: Boolean = false

    @Throws(IllegalStateException::class) constructor(x: Long, f3: Boolean) : this(x){
        this.f3 = f3

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Cons3) return false

        if (this.x != other.x) return false

        if (this.f3 != other.f3) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Cons3@${hashCode().toString(radix=16)}"
        return "Cons3 <x = $x> {f3: $f3}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Cons3) return false

        if (this.f3 != other.f3) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Cons3 {
            var return_object = Cons3(0L)
            return return_object
        }
        fun make(f3: Boolean): HardEnum {
            var return_object = HardEnum.default()
            var inside_object = Cons3.default()
            inside_object.f3 = f3
            return_object.inside = inside_object
            return return_object
        }
    }
}

class Cons4(val x: Long) {
    lateinit var f4: A

    @Throws(IllegalStateException::class) constructor(x: Long, f4: A) : this(x){
        this.f4 = f4

        check()
    }

    fun check() {
        check(this::f4.isInitialized) {"property f4 should be initialized"}

        check(f4.i == 2L) {"dependency i of f4 (is ${f4.i}) should be ${2L}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Cons4) return false

        if (this.x != other.x) return false

        if (this.f4 != other.f4) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Cons4@${hashCode().toString(radix=16)}"
        return "Cons4 <x = $x> {f4: ${f4.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Cons4) return false

        if (this.f4 notSameFields other.f4) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Cons4 {
            var return_object = Cons4(0L)
            return return_object
        }
        fun make(f4: A): HardEnum {
            var return_object = HardEnum.default()
            var inside_object = Cons4.default()
            inside_object.f4 = f4
            return_object.inside = inside_object
            return return_object
        }
    }
}

class HardEnum(val x: Long) {
    lateinit var inside: Any

    @Throws(IllegalStateException::class) constructor(x: Long, inside: Any) : this(x) {
        this.inside = inside
        check()
    }

    fun check() {
        check(this::inside.isInitialized) {"property inside should be initialized"}

        if (x == 1L) {
            if (inside is Cons2) (inside as Cons2).check()
            else if (inside is Cons3) (inside as Cons3).check()
            else check(false) {"not valid inside"}
            return
        }
        if (true) {
            if (inside is Cons4) (inside as Cons4).check()
            else check(false) {"not valid inside"}
            return
        }
        check(false) {"not valid inside"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is HardEnum) return false

        if (inside is Cons2) return (inside as Cons2).equals(other.inside)
        if (inside is Cons3) return (inside as Cons3).equals(other.inside)
        if (inside is Cons4) return (inside as Cons4).equals(other.inside)

        check(false) {"not valid inside"}
        return false
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.HardEnum@${hashCode().toString(radix=16)}"

        if (inside is Cons2) return "(HardEnum) ${(inside as Cons2).toString(depth)}"
        if (inside is Cons3) return "(HardEnum) ${(inside as Cons3).toString(depth)}"
        if (inside is Cons4) return "(HardEnum) ${(inside as Cons4).toString(depth)}"

        check(false) {"not valid inside"}
        return "(HardEnum) Unknow"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is HardEnum) return false

        if (inside is Cons2) return (inside as Cons2).sameFields(other.inside)
        if (inside is Cons3) return (inside as Cons3).sameFields(other.inside)
        if (inside is Cons4) return (inside as Cons4).sameFields(other.inside)

        check(false) {"not valid inside"}
        return false
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : HardEnum {
            var return_object = HardEnum(0L)
            return return_object
        }
    }
}

class HardEnumD(val x: Long, val e: HardEnum) {
    @Throws(IllegalStateException::class) constructor(x: Long, e: HardEnum, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(x, e){
        check()
    }

    fun check() {
        check(e.x == x) {"dependency x of e (is ${e.x}) should be ${x}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is HardEnumD) return false

        if (this.x != other.x) return false
        if (this.e != other.e) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.HardEnumD@${hashCode().toString(radix=16)}"
        return "HardEnumD <x = $x, e = ${e.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is HardEnumD) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : HardEnumD {
            var return_object = HardEnumD(0L, HardEnum.default())
            return return_object
        }
        fun make(): HardEnumD {
            var return_object = HardEnumD.default()
            return return_object
        }
    }
}

class HardCheck() {
    lateinit var v1: HardEnumD
    lateinit var v2: HardEnumD
    lateinit var v3: HardEnumD

    @Throws(IllegalStateException::class) constructor(v1: HardEnumD, v2: HardEnumD, v3: HardEnumD) : this(){
        this.v1 = v1
        this.v2 = v2
        this.v3 = v3

        check()
    }

    fun check() {
        check(this::v1.isInitialized) {"property v1 should be initialized"}
        check(this::v2.isInitialized) {"property v2 should be initialized"}
        check(this::v3.isInitialized) {"property v3 should be initialized"}

        check(v1.x == 1L) {"dependency x of v1 (is ${v1.x}) should be ${1L}"}
        check(v1.e sameFields Cons2.make(f1 = 0L, f2 = "")) {"dependency e of v1 (is ${v1.e}) should be ${Cons2.make(f1 = 0L, f2 = "")}"}
        check(v2.x == 1L) {"dependency x of v2 (is ${v2.x}) should be ${1L}"}
        check(v2.e sameFields Cons3.make(f3 = true)) {"dependency e of v2 (is ${v2.e}) should be ${Cons3.make(f3 = true)}"}
        check(v3.x == 2L) {"dependency x of v3 (is ${v3.x}) should be ${2L}"}
        check(v3.e sameFields Cons4.make(f4 = A.make(x = -1L, y = "soo many strings"))) {"dependency e of v3 (is ${v3.e}) should be ${Cons4.make(f4 = A.make(x = -1L, y = "soo many strings"))}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is HardCheck) return false

        if (this.v1 != other.v1) return false
        if (this.v2 != other.v2) return false
        if (this.v3 != other.v3) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.HardCheck@${hashCode().toString(radix=16)}"
        return "HardCheck {v1: ${v1.toString(depth-1U)}, v2: ${v2.toString(depth-1U)}, v3: ${v3.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is HardCheck) return false

        if (this.v1 notSameFields other.v1) return false
        if (this.v2 notSameFields other.v2) return false
        if (this.v3 notSameFields other.v3) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : HardCheck {
            var return_object = HardCheck()
            return return_object
        }
        fun make(v1: HardEnumD, v2: HardEnumD, v3: HardEnumD): HardCheck {
            var return_object = HardCheck.default()
            return_object.v1 = v1
            return_object.v2 = v2
            return_object.v3 = v3
            return return_object
        }
    }
}

class Cons1(val b: B) {
    @Throws(IllegalStateException::class) constructor(b: B, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(b){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Cons1) return false

        if (this.b != other.b) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Cons1@${hashCode().toString(radix=16)}"
        return "Cons1 <b = ${b.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Cons1) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Cons1 {
            var return_object = Cons1(B.default())
            return return_object
        }
        fun make(): HardConstructedEnumCheck {
            var return_object = HardConstructedEnumCheck.default()
            var inside_object = Cons1.default()
            return_object.inside = inside_object
            return return_object
        }
    }
}

class HardConstructedEnumCheck(val b: B) {
    lateinit var inside: Any

    @Throws(IllegalStateException::class) constructor(b: B, inside: Any) : this(b) {
        this.inside = inside
        check()
    }

    fun check() {
        check(this::inside.isInitialized) {"property inside should be initialized"}

        if (b == B.make(x = 1L, a = A.make(x = 2L, y = "Yay2"))) {
            if (inside is Cons1) (inside as Cons1).check()
            else check(false) {"not valid inside"}
            return
        }
        check(false) {"not valid inside"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is HardConstructedEnumCheck) return false

        if (inside is Cons1) return (inside as Cons1).equals(other.inside)

        check(false) {"not valid inside"}
        return false
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.HardConstructedEnumCheck@${hashCode().toString(radix=16)}"

        if (inside is Cons1) return "(HardConstructedEnumCheck) ${(inside as Cons1).toString(depth)}"

        check(false) {"not valid inside"}
        return "(HardConstructedEnumCheck) Unknow"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is HardConstructedEnumCheck) return false

        if (inside is Cons1) return (inside as Cons1).sameFields(other.inside)

        check(false) {"not valid inside"}
        return false
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : HardConstructedEnumCheck {
            var return_object = HardConstructedEnumCheck(B.default())
            return return_object
        }
    }
}

class HardConstructedMessageCheck() {
    lateinit var bd: BD

    @Throws(IllegalStateException::class) constructor(bd: BD) : this(){
        this.bd = bd

        check()
    }

    fun check() {
        check(this::bd.isInitialized) {"property bd should be initialized"}

        check(bd.b sameFields B.make(x = 5L, a = A.make(x = 2L, y = "Yay"))) {"dependency b of bd (is ${bd.b}) should be ${B.make(x = 5L, a = A.make(x = 2L, y = "Yay"))}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is HardConstructedMessageCheck) return false

        if (this.bd != other.bd) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.HardConstructedMessageCheck@${hashCode().toString(radix=16)}"
        return "HardConstructedMessageCheck {bd: ${bd.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is HardConstructedMessageCheck) return false

        if (this.bd notSameFields other.bd) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : HardConstructedMessageCheck {
            var return_object = HardConstructedMessageCheck()
            return return_object
        }
        fun make(bd: BD): HardConstructedMessageCheck {
            var return_object = HardConstructedMessageCheck.default()
            return_object.bd = bd
            return return_object
        }
    }
}
