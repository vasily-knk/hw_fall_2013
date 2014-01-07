object Test extends App {
    trait Base
    case class Foo(i : Int) extends Base
    case class Bar(s : String) extends Base

    def foo(x : Base) : Int = x match {
        case Foo(i) => i
        case Bar(s) => s.length
    }


}
