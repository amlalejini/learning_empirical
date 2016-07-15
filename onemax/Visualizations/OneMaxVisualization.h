/*
  Visualizations/OneMaxVisualization.h
    This will define a d3 visualization
*/
#ifndef ONEMAXVISUALIZATION_H
#define ONEMAXVISUALIZATION_H

#include <iostream>
#include <sstream>
#include <array>

#include "../../../Empirical/evo/World.h"

#include "../../../Empirical/evo/visualization_utils.h"

#include "../../../d3-emscripten/selection.h"
#include "../../../d3-emscripten/scales.h"
#include "../../../d3-emscripten/axis.h"

struct Margins {
  float top     = 0;
  float bottom  = 0;
  float left    = 0;
  float right   = 0;
};

class OneMaxVisualization : public emp::web::D3Visualization {
  private:
    Margins margins;
    std::array<double, 2> y_domain;
    std::array<double, 2> x_domain;
    std::array<double, 2> x_range;
    std::array<double, 2> y_range;


  public:
    D3::LinearScale x_scale;
    D3::LinearScale y_scale;
    D3::Axis<D3::LinearScale> x_axis;
    D3::Axis<D3::LinearScale> y_axis;

    OneMaxVisualization(int population_size, int genome_size)
      : y_domain{0, (double) population_size},
        x_domain{0, (double) genome_size},
        y_range{0, y_domain[1] * 10},
        x_range{0, x_domain[1] * 10},
        x_scale(),
        y_scale(),
        x_axis("onemax_x"),
        y_axis("onemax_y"),
        D3Visualization(genome_size * 10, population_size * 10, "omv")  // Somewhat frustrating that I can't set width and height later.
    {
    }

    ~OneMaxVisualization() {
    }

    template <typename WORLD>
    void AnimateStep(WORLD &world) {
      // Grab the population canvas
      D3::Selection pop_canvas("#population-canvas");
      // Clear the population canvs out
      pop_canvas.SelectAll("g").Remove();
      for (int p = 0; p < world.GetSize(); p++) {
        // Make a canvas for this organism
        D3::Selection org_canvas = pop_canvas.Append("g").SetAttr("id", "organism-" + std::to_string(p));
        //  This does not work because no .Data for BitVector..
        // org_canvas.SelectAll("rect").Data(world[p].genome);
        // org_canvas.EnterAppend("rect");
        // org_canvas.ExitRemove();
        for (int s = 0; s < world[p].genome.GetSize(); s++) {
          D3::Selection site = org_canvas.Append("rect");
          site.SetAttr("x", x_scale.ApplyScale(s));
          site.SetAttr("y", y_scale.ApplyScale(p));
          site.SetAttr("width", x_scale.ApplyScale(1));
          site.SetAttr("height", y_scale.ApplyScale(1));
          if (world[p].genome[s]) {
            site.SetAttr("fill", "black");
          } else {
            site.SetAttr("fill", "white");
          }
          site.SetAttr("class", "genome-site");
        }
      }
    }

    void Setup() {
      /* Calling this function sets things up for the visualization. */
      std::cout << "Setup is happening..." << std::endl;
      D3::Selection *svg = GetSVG();
      svg->Append("g").SetAttr("id", "population-canvas");
      // Setup axes
      //  x-axis
      x_scale.SetDomain(x_domain);
      x_scale.SetRange(x_range);
      x_axis.SetScale(x_scale);
      //x_axis.SetAttr("class", "axis");
      //  y-axis
      y_scale.SetDomain(y_domain);
      y_scale.SetRange(y_range);
      y_axis.SetScale(y_scale);

      D3::DrawAxes(x_axis, y_axis, *svg);

    }
};



#endif
