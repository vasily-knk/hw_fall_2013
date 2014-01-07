/**
 * Created by vasya on 07.01.14.
 */


trait Graph {
    type Node
    type Edge

    def connect(a: Node, b: Node) : Edge
}

trait GraphImpl extends Graph {
    protected def connectImpl(a: Node, b: Node): Edge
}

trait ListGraph extends GraphImpl {
    private var nodes: List[Node] = Nil
    private var edges: List[Edge] = Nil

    override type Edge = Tuple2[Node, Node]

    protected override def connectImpl(a: Node, b: Node): Edge = {
        val e = (a, b)
        edges ::= e
        e
    }

    protected def addNode(n: Node) = {
        nodes ::= n
    }
}

trait SimpleGraph extends Graph {
    self: GraphImpl =>

    class NodeImpl
    override type Node = NodeImpl
    def makeNode: Node = new NodeImpl
    override def connect(a: Node, b: Node) : Edge = connectImpl(a, b)
}

trait ColoredGraph extends Graph {
    self: GraphImpl =>

    case class SameColor(color: Int) extends Exception

    case class NodeImpl(color: Int)
    override type Node = NodeImpl
    def makeNode(color: Int): Node = NodeImpl(color)

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
        val n2 = cg.makeNode(1)
        val e = cg.connect(n1, n2)
        println(e._1.color)
    }

    main()
}

