# include <Siv3D.hpp>

void Main()
{
	Image image(Window::Size(), Color(0, 0));

	DynamicTexture texture(image);

	Array<Polygon> polygons;

	Array<String> messages;

	Font font(10);

	while (System::Update())
	{
		if (Input::MouseL.pressed)
		{
			const Point from = Input::MouseL.clicked ? Mouse::Pos() : Mouse::PreviousPos();

			Line(from, Mouse::Pos()).overwrite(image, 6, Palette::Green);

			texture.fill(image);
		}
		else if (Input::MouseL.released)
		{
			const Polygon originalPolygon = Imaging::FindExternalContour(image, true);

			if (originalPolygon.num_vertices > 0)
			{
				Polygon polygon = originalPolygon.simplified(1.0);

				Vec2 center = polygon.centroid();

				int iTop = 0;
				int iBottom = 0;

				for (int i = 0; i < polygon.num_vertices; ++i) {
					if (center.distanceFromSq(polygon.vertex(iTop)) < center.distanceFromSq(polygon.vertex(i))) {
						iTop = i;
					}
				}

				polygon = polygon.rotatedAt(center, -Math::Acos((center - polygon.vertex(iTop)).normalized().dot({ 0, 1 })));

				Vec2 topToCenter = (polygon.vertex(iTop) - center).normalized();

				double maxProd = -1;
				for (int i = 0; i < polygon.num_vertices; ++i) {
					double prod = topToCenter.dot((center - polygon.vertex(i)).normalized());
					if (maxProd < prod) {
						iBottom = i;
						maxProd = prod;
					}
				}

				
				Array<Vec2> divided[2];
				for (int i = 0; i < polygon.num_vertices; ++i) {
					if ((iTop - i) * (iBottom - i) >= 0) {
						divided[0].push_back(polygon.vertex(i));
					}
					if ((iTop - i) * (iBottom - i) <= 0) {
						divided[1].push_back(polygon.vertex(i));
					}
				}

				Polygon divdPolygon[2] = {
					Polygon(divided[0]),
					Polygon(divided[1])
				};


				polygons.clear();
				polygons.push_back(divdPolygon[0]);
				polygons.push_back(divdPolygon[1]);

				Vec2 dcenter[2] = { divdPolygon[0].centroid(), divdPolygon[1].centroid() };

				image.fill(Color(0, 0));
				Line(center, center).overwrite(image, 6, Palette::Red);
				Line(polygon.vertex(iTop), polygon.vertex(iTop)).overwrite(image, 6, Palette::Red);
				Line(polygon.vertex(iBottom), polygon.vertex(iBottom)).overwrite(image, 6, Palette::Red);
				Line(divdPolygon[0].centroid(), divdPolygon[0].centroid()).overwrite(image, 6, Palette::Orange);
				Line(divdPolygon[1].centroid(), divdPolygon[1].centroid()).overwrite(image, 6, Palette::Yellow);

				double cscore = dcenter[0].distanceFromSq({ center.x * 2 - dcenter[1].x, dcenter[1].y });
				double rscore = (polygon.vertex(iTop) - center).length() / (center - polygon.vertex(iBottom)).length();

				messages.clear();
				messages.push_back(L"左右対称度：");
				messages.push_back(Format(L"{:.5f} (理想：0)"_fmt, cscore));
				messages.push_back(L"縦比率：");
				messages.push_back(Format(L"{:.5f} (参考黄金比：1.618)"_fmt, rscore));

				texture.fill(image);
			}
		}

		for (const auto& polygon : polygons)
		{
			polygon.draw();

			polygon.drawWireframe(1.0, Palette::Blue);
		}

		for (int i = 0; i < messages.size(); ++i)
		{
			font(messages[i]).draw(0, 20 * i);
		}

		texture.draw();
	}
}