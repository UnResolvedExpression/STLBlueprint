namespace STLParser {

	struct xyz
	{
		double x{};
		double y{};
		double z{};
	};

	class Triangle {
	public:
		Triangle(xyz normal, xyz p1, xyz p2, xyz p3)
			: m_normal{ normal }
			, m_p1{ p1 }
			, m_p2{ p2 }
			, m_p3{ p3 }
		{}

		xyz getNormal() const {
			return m_normal;
		}
		xyz getP1() const {
			return m_p1;
		}
		xyz getP2() const {
			return m_p2;
		}
		xyz getP3() const {
			return m_p3;
		}

	private:
		xyz m_normal{};
		xyz m_p1{};
		xyz m_p2{};
		xyz m_p3{};
	};

	void printXYZ(const xyz& triplit) {
		std::cout << triplit.x << " " << triplit.y << " " << triplit.z << std::endl;
	}

	void printTriangle(const Triangle& triangle) {
		std::cout << "Normal ";
		printXYZ(triangle.getNormal());
		std::cout << "p1 ";
		printXYZ(triangle.getP1());
		std::cout << "p2 ";
		printXYZ(triangle.getP2());
		std::cout << "p3 ";
		printXYZ(triangle.getP3());
	}
}