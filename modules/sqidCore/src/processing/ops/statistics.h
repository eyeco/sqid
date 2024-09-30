/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <processing/op.h>

namespace sqid
{
	namespace Statistics
	{
		class CenterOfMass : public Op
		{
		private:

		protected:
			virtual bool process();

		public:
			CenterOfMass();
			virtual ~CenterOfMass();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Histogram : public Op
		{
		private:
			size_t _bins;
			std::vector<cv::Mat> _histograms;

		protected:
			virtual bool process();

		public:
			explicit Histogram( size_t bins = 16 );
			virtual ~Histogram();

			const std::vector<cv::Mat> &getHistograms() const { return _histograms; }

#ifdef __SUPPORT_GUI
			virtual bool drawUI();

			virtual std::vector<FrameDrawer*> createDrawers();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		//TODO
		//principle component analysis
		// https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#pca
		class PCA : public Op
		{};
	}
}