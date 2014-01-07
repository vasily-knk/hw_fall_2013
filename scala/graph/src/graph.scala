/**
 * Created by vasya on 07.01.14.
 */


trait Graph {
    type Node
    type Edge

    def connect(a: Node, b: Node) : Edge
}

abstract class ListGraph extends Graph {
    private var nodes: List[Node] = Nil
    private var edges: List[Edge] = Nil

    case class EdgeImpl(a: Node, b: Node)
    override type Edge = EdgeImpl

    override def connect(a: Node, b: Node): Edge = {
        val e = EdgeImpl(a, b)
        edges ::= e
        e
    }

    protected def addNode(n: Node) = {
        nodes ::= n
    }
}

class SimpleGraph extends ListGraph {
    class NodeImpl
    override type Node = NodeImpl
    def makeNode: Node = new NodeImpl
}

class ColoredGraph extends ListGraph {
    case class SameColor(color: Int) extends Exception

    case class NodeImpl(color: Int)
    override type Node = NodeImpl
    def makeNode(color: Int): Node = NodeImpl(color)

    override def connect(a: Node, b: Node): Edge = {
        if (a.color == b.color)
            throw SameColor(a.color)
        super.connect(a, b)
    }
}



object Main extends App {
    def main() {
        val cg = new ColoredGraph
        val n1 = cg.makeNode(1)
        val n2 = cg.makeNode(1)
        val e = cg.connect(n1, n2)
    }

    main()
}

