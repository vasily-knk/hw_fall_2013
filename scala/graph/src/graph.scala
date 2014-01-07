import scala.collection.immutable.HashMap


trait Graph {
    type Node
    type Edge

    def connect(a: Node, b: Node) : Edge
}

trait GraphImpl extends Graph {
    protected def connectImpl(a: Node, b: Node): Edge
    protected def addNode(n: Node) : Node
}

trait ListGraph extends GraphImpl {
    private var nodes: HashMap[Node, List[Node]] = HashMap[Node, List[Node]]()

    override type Edge = Tuple2[Node, Node]

    protected override def connectImpl(a: Node, b: Node): Edge = {
        nodes += a -> (nodes.get(a).get :+ b)
        (a, b)
    }

    protected def addNode(n: Node) : Node = {
        nodes += (n -> Nil)
        n
    }
}

trait BiDirListGraph extends ListGraph {
    protected override def connectImpl(a: Node, b: Node): Edge = {
        super.connectImpl(a, b)
        super.connectImpl(b, a)
    }
}

trait SimpleGraph extends Graph {
    self: GraphImpl =>

    class NodeImpl
    override type Node = NodeImpl
    def makeNode: Node = addNode(new NodeImpl)
    override def connect(a: Node, b: Node) : Edge = connectImpl(a, b)
}

trait ColoredGraph extends Graph {
    self: GraphImpl =>

    case class SameColor(color: Int) extends Exception

    case class NodeImpl(color: Int)
    override type Node = NodeImpl
    def makeNode(color: Int): Node = addNode(NodeImpl(color))

    override def connect(a: Node, b: Node): Edge = {
        if (a.color == b.color)
            throw SameColor(a.color)
        connectImpl(a, b)
    }
}



object Main extends App {
    def main() {
        val cg = new ColoredGraph with ListGraph
        val n1 = cg.makeNode(1)
        val n2 = cg.makeNode(2)
        val e = cg.connect(n1, n2)
        println(e._1.color)
    }

    main()
}

