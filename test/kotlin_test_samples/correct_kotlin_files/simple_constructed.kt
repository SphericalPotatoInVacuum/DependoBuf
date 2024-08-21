package dbuf

// This file is autogenerated. Please, do not change it manually.

class Constructable() {
    var f1: Long = 0L
    var f2: String = ""
    var f3: Boolean = false
    var f4: Long = 0L

    fun check() {
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
    fun check() {
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

    fun check() {
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

    fun check() {
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
    fun check() {
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
    fun check() {
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

    fun check() {
        check(this::inside.isInitialized) {"property inside should be initialized"}

        if (s == Simple.make(f1 = 2L)) {
            if (inside is Ctr1) {
                (inside as Ctr1).check()
            }
            else {
                check(false) {"not valid inside"}
            }
            return
        }
        if (s == Simple.make(f1 = (x + x))) {
            if (inside is Ctr2) {
                (inside as Ctr2).check()
            }
            else {
                check(false) {"not valid inside"}
            }
            return
        }
        if (true) {
            if (inside is Ctr3) {
                (inside as Ctr3).check()
            }
            else {
                check(false) {"not valid inside"}
            }
            return
        }
        check(false) {"not valid inside"}
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

    fun check() {
        check(this::c.isInitialized) {"property c should be initialized"}

        check(c.c == Constructable.make(f1 = v1, f2 = "i am second", f3 = f1, f4 = (1L - 2L))) {"dependency c of c (is ${c.c}) should be ${Constructable.make(f1 = v1, f2 = "i am second", f3 = f1, f4 = (1L - 2L))}"}
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

