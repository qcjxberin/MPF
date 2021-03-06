﻿using MPF.Data;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace MPF.Media
{
    public class GeometryDrawing : Drawing
    {
        public DependencyProperty<Geometry> GeometryProperty = DependencyProperty.Register(nameof(Geometry), typeof(GeometryDrawing),
            new PropertyMetadata<Geometry>(Geometry.Empty));

        public Geometry Geometry
        {
            get { return GetValue(GeometryProperty); }
            set { this.SetLocalValue(GeometryProperty, value); }
        }

        public GeometryDrawing()
        {

        }
    }
}
