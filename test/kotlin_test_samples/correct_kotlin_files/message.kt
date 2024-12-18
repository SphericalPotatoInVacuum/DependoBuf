package dbuf

// This file is autogenerated. Please, do not change it manually.

class IntD(val i: Long) {
    var x: Boolean = false

    @Throws(IllegalStateException::class) constructor(i: Long, x: Boolean) : this(i){
        this.x = x

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is IntD) return false

        if (this.i != other.i) return false

        if (this.x != other.x) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.IntD@${hashCode().toString(radix=16)}"
        return "IntD <i = $i> {x: $x}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is IntD) return false

        if (this.x != other.x) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : IntD {
            var return_object = IntD(0L)
            return return_object
        }
        fun make(x: Boolean): IntD {
            var return_object = IntD.default()
            return_object.x = x
            return return_object
        }
    }
}

class BinaryMathCheck(val a: Long, val b: Long, val v1: IntD, val v2: IntD, val v3: IntD) {
    var f1: Long = 0L
    var f2: Long = 0L
    lateinit var f3: IntD
    lateinit var f4: IntD
    lateinit var f5: IntD
    lateinit var f6: IntD

    @Throws(IllegalStateException::class) constructor(a: Long, b: Long, v1: IntD, v2: IntD, v3: IntD, f1: Long, f2: Long, f3: IntD, f4: IntD, f5: IntD, f6: IntD) : this(a, b, v1, v2, v3){
        this.f1 = f1
        this.f2 = f2
        this.f3 = f3
        this.f4 = f4
        this.f5 = f5
        this.f6 = f6

        check()
    }

    fun check() {
        check(v1.i == (a + b)) {"dependency i of v1 (is ${v1.i}) should be ${(a + b)}"}
        check(v2.i == (a - b)) {"dependency i of v2 (is ${v2.i}) should be ${(a - b)}"}
        check(v3.i == (a * b)) {"dependency i of v3 (is ${v3.i}) should be ${(a * b)}"}

        check(this::f3.isInitialized) {"property f3 should be initialized"}
        check(this::f4.isInitialized) {"property f4 should be initialized"}
        check(this::f5.isInitialized) {"property f5 should be initialized"}
        check(this::f6.isInitialized) {"property f6 should be initialized"}

        check(f3.i == (f1 + a)) {"dependency i of f3 (is ${f3.i}) should be ${(f1 + a)}"}
        check(f4.i == (f1 + f2)) {"dependency i of f4 (is ${f4.i}) should be ${(f1 + f2)}"}
        check(f5.i == (f1 - f2)) {"dependency i of f5 (is ${f5.i}) should be ${(f1 - f2)}"}
        check(f6.i == (f1 * f2)) {"dependency i of f6 (is ${f6.i}) should be ${(f1 * f2)}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is BinaryMathCheck) return false

        if (this.a != other.a) return false
        if (this.b != other.b) return false
        if (this.v1 != other.v1) return false
        if (this.v2 != other.v2) return false
        if (this.v3 != other.v3) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false
        if (this.f5 != other.f5) return false
        if (this.f6 != other.f6) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.BinaryMathCheck@${hashCode().toString(radix=16)}"
        return "BinaryMathCheck <a = $a, b = $b, v1 = ${v1.toString(depth-1U)}, v2 = ${v2.toString(depth-1U)}, v3 = ${v3.toString(depth-1U)}> {f1: $f1, f2: $f2, f3: ${f3.toString(depth-1U)}, f4: ${f4.toString(depth-1U)}, f5: ${f5.toString(depth-1U)}, f6: ${f6.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is BinaryMathCheck) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 notSameFields other.f3) return false
        if (this.f4 notSameFields other.f4) return false
        if (this.f5 notSameFields other.f5) return false
        if (this.f6 notSameFields other.f6) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : BinaryMathCheck {
            var return_object = BinaryMathCheck(0L, 0L, IntD.default(), IntD.default(), IntD.default())
            return return_object
        }
        fun make(f1: Long, f2: Long, f3: IntD, f4: IntD, f5: IntD, f6: IntD): BinaryMathCheck {
            var return_object = BinaryMathCheck.default()
            return_object.f1 = f1
            return_object.f2 = f2
            return_object.f3 = f3
            return_object.f4 = f4
            return_object.f5 = f5
            return_object.f6 = f6
            return return_object
        }
    }
}

class BoolD(val b: Boolean) {
    var x: String = ""

    @Throws(IllegalStateException::class) constructor(b: Boolean, x: String) : this(b){
        this.x = x

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is BoolD) return false

        if (this.b != other.b) return false

        if (this.x != other.x) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.BoolD@${hashCode().toString(radix=16)}"
        return "BoolD <b = $b> {x: $x}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is BoolD) return false

        if (this.x != other.x) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : BoolD {
            var return_object = BoolD(false)
            return return_object
        }
        fun make(x: String): BoolD {
            var return_object = BoolD.default()
            return_object.x = x
            return return_object
        }
    }
}

class StringD(val s: String) {
    var x: Long = 0L

    @Throws(IllegalStateException::class) constructor(s: String, x: Long) : this(s){
        this.x = x

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is StringD) return false

        if (this.s != other.s) return false

        if (this.x != other.x) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.StringD@${hashCode().toString(radix=16)}"
        return "StringD <s = $s> {x: $x}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is StringD) return false

        if (this.x != other.x) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : StringD {
            var return_object = StringD("")
            return return_object
        }
        fun make(x: Long): StringD {
            var return_object = StringD.default()
            return_object.x = x
            return return_object
        }
    }
}

class ConstantCheck(val v1: IntD, val v2: IntD, val v3: IntD, val v4: BoolD, val v5: BoolD, val v6: StringD, val v7: StringD) {
    lateinit var f1: IntD
    lateinit var f2: IntD
    lateinit var f3: IntD
    lateinit var f4: BoolD
    lateinit var f5: BoolD
    lateinit var f6: StringD
    lateinit var f7: StringD

    @Throws(IllegalStateException::class) constructor(v1: IntD, v2: IntD, v3: IntD, v4: BoolD, v5: BoolD, v6: StringD, v7: StringD, f1: IntD, f2: IntD, f3: IntD, f4: BoolD, f5: BoolD, f6: StringD, f7: StringD) : this(v1, v2, v3, v4, v5, v6, v7){
        this.f1 = f1
        this.f2 = f2
        this.f3 = f3
        this.f4 = f4
        this.f5 = f5
        this.f6 = f6
        this.f7 = f7

        check()
    }

    fun check() {
        check(v1.i == 12L) {"dependency i of v1 (is ${v1.i}) should be ${12L}"}
        check(v2.i == -12L) {"dependency i of v2 (is ${v2.i}) should be ${-12L}"}
        check(v3.i == 0L) {"dependency i of v3 (is ${v3.i}) should be ${0L}"}
        check(v4.b == true) {"dependency b of v4 (is ${v4.b}) should be ${true}"}
        check(v5.b == false) {"dependency b of v5 (is ${v5.b}) should be ${false}"}
        check(v6.s == "hi there!") {"dependency s of v6 (is ${v6.s}) should be ${"hi there!"}"}
        check(v7.s == "") {"dependency s of v7 (is ${v7.s}) should be ${""}"}

        check(this::f1.isInitialized) {"property f1 should be initialized"}
        check(this::f2.isInitialized) {"property f2 should be initialized"}
        check(this::f3.isInitialized) {"property f3 should be initialized"}
        check(this::f4.isInitialized) {"property f4 should be initialized"}
        check(this::f5.isInitialized) {"property f5 should be initialized"}
        check(this::f6.isInitialized) {"property f6 should be initialized"}
        check(this::f7.isInitialized) {"property f7 should be initialized"}

        check(f1.i == 13L) {"dependency i of f1 (is ${f1.i}) should be ${13L}"}
        check(f2.i == -13L) {"dependency i of f2 (is ${f2.i}) should be ${-13L}"}
        check(f3.i == 0L) {"dependency i of f3 (is ${f3.i}) should be ${0L}"}
        check(f4.b == true) {"dependency b of f4 (is ${f4.b}) should be ${true}"}
        check(f5.b == false) {"dependency b of f5 (is ${f5.b}) should be ${false}"}
        check(f6.s == "hey") {"dependency s of f6 (is ${f6.s}) should be ${"hey"}"}
        check(f7.s == "") {"dependency s of f7 (is ${f7.s}) should be ${""}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is ConstantCheck) return false

        if (this.v1 != other.v1) return false
        if (this.v2 != other.v2) return false
        if (this.v3 != other.v3) return false
        if (this.v4 != other.v4) return false
        if (this.v5 != other.v5) return false
        if (this.v6 != other.v6) return false
        if (this.v7 != other.v7) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false
        if (this.f5 != other.f5) return false
        if (this.f6 != other.f6) return false
        if (this.f7 != other.f7) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.ConstantCheck@${hashCode().toString(radix=16)}"
        return "ConstantCheck <v1 = ${v1.toString(depth-1U)}, v2 = ${v2.toString(depth-1U)}, v3 = ${v3.toString(depth-1U)}, v4 = ${v4.toString(depth-1U)}, v5 = ${v5.toString(depth-1U)}, v6 = ${v6.toString(depth-1U)}, v7 = ${v7.toString(depth-1U)}> {f1: ${f1.toString(depth-1U)}, f2: ${f2.toString(depth-1U)}, f3: ${f3.toString(depth-1U)}, f4: ${f4.toString(depth-1U)}, f5: ${f5.toString(depth-1U)}, f6: ${f6.toString(depth-1U)}, f7: ${f7.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is ConstantCheck) return false

        if (this.f1 notSameFields other.f1) return false
        if (this.f2 notSameFields other.f2) return false
        if (this.f3 notSameFields other.f3) return false
        if (this.f4 notSameFields other.f4) return false
        if (this.f5 notSameFields other.f5) return false
        if (this.f6 notSameFields other.f6) return false
        if (this.f7 notSameFields other.f7) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : ConstantCheck {
            var return_object = ConstantCheck(IntD.default(), IntD.default(), IntD.default(), BoolD.default(), BoolD.default(), StringD.default(), StringD.default())
            return return_object
        }
        fun make(f1: IntD, f2: IntD, f3: IntD, f4: BoolD, f5: BoolD, f6: StringD, f7: StringD): ConstantCheck {
            var return_object = ConstantCheck.default()
            return_object.f1 = f1
            return_object.f2 = f2
            return_object.f3 = f3
            return_object.f4 = f4
            return_object.f5 = f5
            return_object.f6 = f6
            return_object.f7 = f7
            return return_object
        }
    }
}

class DeepDependency(val a: Long) {
    @Throws(IllegalStateException::class) constructor(a: Long, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(a){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is DeepDependency) return false

        if (this.a != other.a) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.DeepDependency@${hashCode().toString(radix=16)}"
        return "DeepDependency <a = $a> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is DeepDependency) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : DeepDependency {
            var return_object = DeepDependency(0L)
            return return_object
        }
        fun make(): DeepDependency {
            var return_object = DeepDependency.default()
            return return_object
        }
    }
}

class DeepDependency2(val a: Long, val b: DeepDependency) {
    @Throws(IllegalStateException::class) constructor(a: Long, b: DeepDependency, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(a, b){
        check()
    }

    fun check() {
        check(b.a == a) {"dependency a of b (is ${b.a}) should be ${a}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is DeepDependency2) return false

        if (this.a != other.a) return false
        if (this.b != other.b) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.DeepDependency2@${hashCode().toString(radix=16)}"
        return "DeepDependency2 <a = $a, b = ${b.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is DeepDependency2) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : DeepDependency2 {
            var return_object = DeepDependency2(0L, DeepDependency.default())
            return return_object
        }
        fun make(): DeepDependency2 {
            var return_object = DeepDependency2.default()
            return return_object
        }
    }
}

class DeepDependency3(val a: Long, val b: DeepDependency, val c: DeepDependency2) {
    @Throws(IllegalStateException::class) constructor(a: Long, b: DeepDependency, c: DeepDependency2, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(a, b, c){
        check()
    }

    fun check() {
        check(b.a == a) {"dependency a of b (is ${b.a}) should be ${a}"}
        check(c.a == a) {"dependency a of c (is ${c.a}) should be ${a}"}
        check(c.b sameFields b) {"dependency b of c (is ${c.b}) should be ${b}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is DeepDependency3) return false

        if (this.a != other.a) return false
        if (this.b != other.b) return false
        if (this.c != other.c) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.DeepDependency3@${hashCode().toString(radix=16)}"
        return "DeepDependency3 <a = $a, b = ${b.toString(depth-1U)}, c = ${c.toString(depth-1U)}> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is DeepDependency3) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : DeepDependency3 {
            var return_object = DeepDependency3(0L, DeepDependency.default(), DeepDependency2.default())
            return return_object
        }
        fun make(): DeepDependency3 {
            var return_object = DeepDependency3.default()
            return return_object
        }
    }
}

class DefaultFields() {
    var x: Long = 0L
    var b: Boolean = false
    var s: String = ""

    @Throws(IllegalStateException::class) constructor(x: Long, b: Boolean, s: String) : this(){
        this.x = x
        this.b = b
        this.s = s

        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is DefaultFields) return false

        if (this.x != other.x) return false
        if (this.b != other.b) return false
        if (this.s != other.s) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.DefaultFields@${hashCode().toString(radix=16)}"
        return "DefaultFields {x: $x, b: $b, s: $s}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is DefaultFields) return false

        if (this.x != other.x) return false
        if (this.b != other.b) return false
        if (this.s != other.s) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : DefaultFields {
            var return_object = DefaultFields()
            return return_object
        }
        fun make(x: Long, b: Boolean, s: String): DefaultFields {
            var return_object = DefaultFields.default()
            return_object.x = x
            return_object.b = b
            return_object.s = s
            return return_object
        }
    }
}

class DependCheck(val x: Long, val i: IntD, val b: BoolD, val s: StringD, val i2: IntD) {
    var f1: Long = 0L
    lateinit var f2: IntD
    lateinit var f3: BoolD
    lateinit var f4: StringD
    lateinit var f5: IntD

    @Throws(IllegalStateException::class) constructor(x: Long, i: IntD, b: BoolD, s: StringD, i2: IntD, f1: Long, f2: IntD, f3: BoolD, f4: StringD, f5: IntD) : this(x, i, b, s, i2){
        this.f1 = f1
        this.f2 = f2
        this.f3 = f3
        this.f4 = f4
        this.f5 = f5

        check()
    }

    fun check() {
        check(i.i == x) {"dependency i of i (is ${i.i}) should be ${x}"}
        check(b.b == i.x) {"dependency b of b (is ${b.b}) should be ${i.x}"}
        check(s.s == b.x) {"dependency s of s (is ${s.s}) should be ${b.x}"}
        check(i2.i == s.x) {"dependency i of i2 (is ${i2.i}) should be ${s.x}"}

        check(this::f2.isInitialized) {"property f2 should be initialized"}
        check(this::f3.isInitialized) {"property f3 should be initialized"}
        check(this::f4.isInitialized) {"property f4 should be initialized"}
        check(this::f5.isInitialized) {"property f5 should be initialized"}

        check(f2.i == f1) {"dependency i of f2 (is ${f2.i}) should be ${f1}"}
        check(f3.b == f2.x) {"dependency b of f3 (is ${f3.b}) should be ${f2.x}"}
        check(f4.s == f3.x) {"dependency s of f4 (is ${f4.s}) should be ${f3.x}"}
        check(f5.i == f4.x) {"dependency i of f5 (is ${f5.i}) should be ${f4.x}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is DependCheck) return false

        if (this.x != other.x) return false
        if (this.i != other.i) return false
        if (this.b != other.b) return false
        if (this.s != other.s) return false
        if (this.i2 != other.i2) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false
        if (this.f5 != other.f5) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.DependCheck@${hashCode().toString(radix=16)}"
        return "DependCheck <x = $x, i = ${i.toString(depth-1U)}, b = ${b.toString(depth-1U)}, s = ${s.toString(depth-1U)}, i2 = ${i2.toString(depth-1U)}> {f1: $f1, f2: ${f2.toString(depth-1U)}, f3: ${f3.toString(depth-1U)}, f4: ${f4.toString(depth-1U)}, f5: ${f5.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is DependCheck) return false

        if (this.f1 != other.f1) return false
        if (this.f2 notSameFields other.f2) return false
        if (this.f3 notSameFields other.f3) return false
        if (this.f4 notSameFields other.f4) return false
        if (this.f5 notSameFields other.f5) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : DependCheck {
            var return_object = DependCheck(0L, IntD.default(), BoolD.default(), StringD.default(), IntD.default())
            return return_object
        }
        fun make(f1: Long, f2: IntD, f3: BoolD, f4: StringD, f5: IntD): DependCheck {
            var return_object = DependCheck.default()
            return_object.f1 = f1
            return_object.f2 = f2
            return_object.f3 = f3
            return_object.f4 = f4
            return_object.f5 = f5
            return return_object
        }
    }
}

class Empty() {
    @Throws(IllegalStateException::class) constructor(@Suppress("UNUSED_PARAMETER") _unused: Any) : this(){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is Empty) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.Empty@${hashCode().toString(radix=16)}"
        return "Empty {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is Empty) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : Empty {
            var return_object = Empty()
            return return_object
        }
        fun make(): Empty {
            var return_object = Empty.default()
            return return_object
        }
    }
}

class ManyDepend(val a: Long, val b: String, val c: Long) {
    @Throws(IllegalStateException::class) constructor(a: Long, b: String, c: Long, @Suppress("UNUSED_PARAMETER") _unused: Any) : this(a, b, c){
        check()
    }

    fun check() {
    }

    override fun equals(other: Any?): Boolean {
        if (other !is ManyDepend) return false

        if (this.a != other.a) return false
        if (this.b != other.b) return false
        if (this.c != other.c) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.ManyDepend@${hashCode().toString(radix=16)}"
        return "ManyDepend <a = $a, b = $b, c = $c> {}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is ManyDepend) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : ManyDepend {
            var return_object = ManyDepend(0L, "", 0L)
            return return_object
        }
        fun make(): ManyDepend {
            var return_object = ManyDepend.default()
            return return_object
        }
    }
}

class ManyDependCheck(val a: Long, val b: Boolean, val c: BoolD, val many: ManyDepend) {
    var f1: Long = 0L
    var f2: Long = 0L
    var f3: Boolean = false
    var f4: String = ""
    lateinit var f5: ManyDepend
    lateinit var f6: ManyDepend

    @Throws(IllegalStateException::class) constructor(a: Long, b: Boolean, c: BoolD, many: ManyDepend, f1: Long, f2: Long, f3: Boolean, f4: String, f5: ManyDepend, f6: ManyDepend) : this(a, b, c, many){
        this.f1 = f1
        this.f2 = f2
        this.f3 = f3
        this.f4 = f4
        this.f5 = f5
        this.f6 = f6

        check()
    }

    fun check() {
        check(c.b == b) {"dependency b of c (is ${c.b}) should be ${b}"}
        check(many.a == a) {"dependency a of many (is ${many.a}) should be ${a}"}
        check(many.b == c.x) {"dependency b of many (is ${many.b}) should be ${c.x}"}
        check(many.c == a) {"dependency c of many (is ${many.c}) should be ${a}"}

        check(this::f5.isInitialized) {"property f5 should be initialized"}
        check(this::f6.isInitialized) {"property f6 should be initialized"}

        check(f5.a == a) {"dependency a of f5 (is ${f5.a}) should be ${a}"}
        check(f5.b == c.x) {"dependency b of f5 (is ${f5.b}) should be ${c.x}"}
        check(f5.c == a) {"dependency c of f5 (is ${f5.c}) should be ${a}"}
        check(f6.a == (a + f1)) {"dependency a of f6 (is ${f6.a}) should be ${(a + f1)}"}
        check(f6.b == (f4 + c.x)) {"dependency b of f6 (is ${f6.b}) should be ${(f4 + c.x)}"}
        check(f6.c == 12L) {"dependency c of f6 (is ${f6.c}) should be ${12L}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is ManyDependCheck) return false

        if (this.a != other.a) return false
        if (this.b != other.b) return false
        if (this.c != other.c) return false
        if (this.many != other.many) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false
        if (this.f5 != other.f5) return false
        if (this.f6 != other.f6) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.ManyDependCheck@${hashCode().toString(radix=16)}"
        return "ManyDependCheck <a = $a, b = $b, c = ${c.toString(depth-1U)}, many = ${many.toString(depth-1U)}> {f1: $f1, f2: $f2, f3: $f3, f4: $f4, f5: ${f5.toString(depth-1U)}, f6: ${f6.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is ManyDependCheck) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false
        if (this.f5 notSameFields other.f5) return false
        if (this.f6 notSameFields other.f6) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : ManyDependCheck {
            var return_object = ManyDependCheck(0L, false, BoolD.default(), ManyDepend.default())
            return return_object
        }
        fun make(f1: Long, f2: Long, f3: Boolean, f4: String, f5: ManyDepend, f6: ManyDepend): ManyDependCheck {
            var return_object = ManyDependCheck.default()
            return_object.f1 = f1
            return_object.f2 = f2
            return_object.f3 = f3
            return_object.f4 = f4
            return_object.f5 = f5
            return_object.f6 = f6
            return return_object
        }
    }
}

class OperationOrderCheck(val a: Long, val b: Long, val c: IntD) {
    var f1: Long = 0L
    var f2: Long = 0L
    var f3: Long = 0L
    var f4: Long = 0L
    var f5: Long = 0L
    var f6: Long = 0L
    var f7: Long = 0L
    lateinit var f8: IntD

    @Throws(IllegalStateException::class) constructor(a: Long, b: Long, c: IntD, f1: Long, f2: Long, f3: Long, f4: Long, f5: Long, f6: Long, f7: Long, f8: IntD) : this(a, b, c){
        this.f1 = f1
        this.f2 = f2
        this.f3 = f3
        this.f4 = f4
        this.f5 = f5
        this.f6 = f6
        this.f7 = f7
        this.f8 = f8

        check()
    }

    fun check() {
        check(c.i == ((((((a + (b * -(a))) + b) + ((a * (5L + 1L)) * b)) - a) + 12L) - -3L)) {"dependency i of c (is ${c.i}) should be ${((((((a + (b * -(a))) + b) + ((a * (5L + 1L)) * b)) - a) + 12L) - -3L)}"}

        check(this::f8.isInitialized) {"property f8 should be initialized"}

        check(f8.i == (((f1 + (f2 * f3)) + f4) - (f5 * (f6 + -(f7))))) {"dependency i of f8 (is ${f8.i}) should be ${(((f1 + (f2 * f3)) + f4) - (f5 * (f6 + -(f7))))}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is OperationOrderCheck) return false

        if (this.a != other.a) return false
        if (this.b != other.b) return false
        if (this.c != other.c) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false
        if (this.f5 != other.f5) return false
        if (this.f6 != other.f6) return false
        if (this.f7 != other.f7) return false
        if (this.f8 != other.f8) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.OperationOrderCheck@${hashCode().toString(radix=16)}"
        return "OperationOrderCheck <a = $a, b = $b, c = ${c.toString(depth-1U)}> {f1: $f1, f2: $f2, f3: $f3, f4: $f4, f5: $f5, f6: $f6, f7: $f7, f8: ${f8.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is OperationOrderCheck) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false
        if (this.f5 != other.f5) return false
        if (this.f6 != other.f6) return false
        if (this.f7 != other.f7) return false
        if (this.f8 notSameFields other.f8) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : OperationOrderCheck {
            var return_object = OperationOrderCheck(0L, 0L, IntD.default())
            return return_object
        }
        fun make(f1: Long, f2: Long, f3: Long, f4: Long, f5: Long, f6: Long, f7: Long, f8: IntD): OperationOrderCheck {
            var return_object = OperationOrderCheck.default()
            return_object.f1 = f1
            return_object.f2 = f2
            return_object.f3 = f3
            return_object.f4 = f4
            return_object.f5 = f5
            return_object.f6 = f6
            return_object.f7 = f7
            return_object.f8 = f8
            return return_object
        }
    }
}

class StringCheck(val a: String, val b: String, val c: StringD) {
    var f1: String = ""
    var f2: String = ""
    lateinit var f3: StringD

    @Throws(IllegalStateException::class) constructor(a: String, b: String, c: StringD, f1: String, f2: String, f3: StringD) : this(a, b, c){
        this.f1 = f1
        this.f2 = f2
        this.f3 = f3

        check()
    }

    fun check() {
        check(c.s == (((a + b) + b) + a)) {"dependency s of c (is ${c.s}) should be ${(((a + b) + b) + a)}"}

        check(this::f3.isInitialized) {"property f3 should be initialized"}

        check(f3.s == (((a + f1) + f2) + b)) {"dependency s of f3 (is ${f3.s}) should be ${(((a + f1) + f2) + b)}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is StringCheck) return false

        if (this.a != other.a) return false
        if (this.b != other.b) return false
        if (this.c != other.c) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.StringCheck@${hashCode().toString(radix=16)}"
        return "StringCheck <a = $a, b = $b, c = ${c.toString(depth-1U)}> {f1: $f1, f2: $f2, f3: ${f3.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is StringCheck) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 notSameFields other.f3) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : StringCheck {
            var return_object = StringCheck("", "", StringD.default())
            return return_object
        }
        fun make(f1: String, f2: String, f3: StringD): StringCheck {
            var return_object = StringCheck.default()
            return_object.f1 = f1
            return_object.f2 = f2
            return_object.f3 = f3
            return return_object
        }
    }
}

class UnaryCheck(val a: Long, val b: Boolean, val v1: IntD, val v2: IntD, val v3: BoolD, val v4: BoolD) {
    var f1: Long = 0L
    var f2: Boolean = false
    lateinit var f3: IntD
    lateinit var f4: IntD
    lateinit var f5: IntD
    lateinit var f6: BoolD
    lateinit var f7: BoolD
    lateinit var f8: BoolD

    @Throws(IllegalStateException::class) constructor(a: Long, b: Boolean, v1: IntD, v2: IntD, v3: BoolD, v4: BoolD, f1: Long, f2: Boolean, f3: IntD, f4: IntD, f5: IntD, f6: BoolD, f7: BoolD, f8: BoolD) : this(a, b, v1, v2, v3, v4){
        this.f1 = f1
        this.f2 = f2
        this.f3 = f3
        this.f4 = f4
        this.f5 = f5
        this.f6 = f6
        this.f7 = f7
        this.f8 = f8

        check()
    }

    fun check() {
        check(v1.i == -(a)) {"dependency i of v1 (is ${v1.i}) should be ${-(a)}"}
        check(v2.i == -(-(-(-(-(a)))))) {"dependency i of v2 (is ${v2.i}) should be ${-(-(-(-(-(a)))))}"}
        check(v3.b == !(b)) {"dependency b of v3 (is ${v3.b}) should be ${!(b)}"}
        check(v4.b == !(!(!(!(!(b)))))) {"dependency b of v4 (is ${v4.b}) should be ${!(!(!(!(!(b)))))}"}

        check(this::f3.isInitialized) {"property f3 should be initialized"}
        check(this::f4.isInitialized) {"property f4 should be initialized"}
        check(this::f5.isInitialized) {"property f5 should be initialized"}
        check(this::f6.isInitialized) {"property f6 should be initialized"}
        check(this::f7.isInitialized) {"property f7 should be initialized"}
        check(this::f8.isInitialized) {"property f8 should be initialized"}

        check(f3.i == -(a)) {"dependency i of f3 (is ${f3.i}) should be ${-(a)}"}
        check(f4.i == -(f1)) {"dependency i of f4 (is ${f4.i}) should be ${-(f1)}"}
        check(f5.i == -(-(-(f1)))) {"dependency i of f5 (is ${f5.i}) should be ${-(-(-(f1)))}"}
        check(f6.b == !(b)) {"dependency b of f6 (is ${f6.b}) should be ${!(b)}"}
        check(f7.b == !(f2)) {"dependency b of f7 (is ${f7.b}) should be ${!(f2)}"}
        check(f8.b == !(!(!(f2)))) {"dependency b of f8 (is ${f8.b}) should be ${!(!(!(f2)))}"}
    }

    override fun equals(other: Any?): Boolean {
        if (other !is UnaryCheck) return false

        if (this.a != other.a) return false
        if (this.b != other.b) return false
        if (this.v1 != other.v1) return false
        if (this.v2 != other.v2) return false
        if (this.v3 != other.v3) return false
        if (this.v4 != other.v4) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 != other.f3) return false
        if (this.f4 != other.f4) return false
        if (this.f5 != other.f5) return false
        if (this.f6 != other.f6) return false
        if (this.f7 != other.f7) return false
        if (this.f8 != other.f8) return false

        return true
    }

    override fun toString(): String {
        return toString(1U)
    }

    fun toString(depth: UInt): String {
        if (depth == 0U) return "dbuf.UnaryCheck@${hashCode().toString(radix=16)}"
        return "UnaryCheck <a = $a, b = $b, v1 = ${v1.toString(depth-1U)}, v2 = ${v2.toString(depth-1U)}, v3 = ${v3.toString(depth-1U)}, v4 = ${v4.toString(depth-1U)}> {f1: $f1, f2: $f2, f3: ${f3.toString(depth-1U)}, f4: ${f4.toString(depth-1U)}, f5: ${f5.toString(depth-1U)}, f6: ${f6.toString(depth-1U)}, f7: ${f7.toString(depth-1U)}, f8: ${f8.toString(depth-1U)}}"
    }

    infix internal fun sameFields(other: Any?): Boolean {
        if (other !is UnaryCheck) return false

        if (this.f1 != other.f1) return false
        if (this.f2 != other.f2) return false
        if (this.f3 notSameFields other.f3) return false
        if (this.f4 notSameFields other.f4) return false
        if (this.f5 notSameFields other.f5) return false
        if (this.f6 notSameFields other.f6) return false
        if (this.f7 notSameFields other.f7) return false
        if (this.f8 notSameFields other.f8) return false

        return true
    }

    infix internal fun notSameFields(other: Any?): Boolean {
        return !(this sameFields other)
    }

    internal companion object Factory {
        fun default() : UnaryCheck {
            var return_object = UnaryCheck(0L, false, IntD.default(), IntD.default(), BoolD.default(), BoolD.default())
            return return_object
        }
        fun make(f1: Long, f2: Boolean, f3: IntD, f4: IntD, f5: IntD, f6: BoolD, f7: BoolD, f8: BoolD): UnaryCheck {
            var return_object = UnaryCheck.default()
            return_object.f1 = f1
            return_object.f2 = f2
            return_object.f3 = f3
            return_object.f4 = f4
            return_object.f5 = f5
            return_object.f6 = f6
            return_object.f7 = f7
            return_object.f8 = f8
            return return_object
        }
    }
}

