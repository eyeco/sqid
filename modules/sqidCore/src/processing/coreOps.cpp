/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "coreOps.h"

#include <config.h>

#include "ops/general.h"
#include "ops/math.h"
#include "ops/pointClouds.h"
#include "ops/statistics.h"
#include "ops/temporal.h"
#include "ops/tracking.h"
#include "ops/util.h"
#include "ops/ocv.h"
#include "ops/sink.h"
#include "ops/sensor.h"
#include "ops/serial.h"
#include "ops/file.h"
#include "ops/osc.h"
#include "ops/color.h"
#include "ops/imaging.h"

#include <processing/opFactory.h>

namespace sqid
{
	bool initCoreOps()
	{
		//general
		REGISTER_OP_TYPE( General::NOP );
		REGISTER_OP_TYPE( General::Const );
		REGISTER_OP_TYPE( General::Noise );
		REGISTER_OP_TYPE( General::Signal );
		REGISTER_OP_TYPE( General::ContourDetector );

		//util
		REGISTER_OP_TYPE( Util::Time );
		REGISTER_OP_TYPE( Util::TimeStamp );
		REGISTER_OP_TYPE( Util::Sync );
		REGISTER_OP_TYPE( Util::Sampler );
		REGISTER_OP_TYPE( Util::Buffer );
		REGISTER_OP_TYPE( Util::Split );
		REGISTER_OP_TYPE( Util::Merge );
		REGISTER_OP_TYPE( Util::Crop );
		REGISTER_OP_TYPE( Util::Resize );
		REGISTER_OP_TYPE( Util::Join );
		REGISTER_OP_TYPE( Util::Transpose );
		REGISTER_OP_TYPE( Util::Flatten );
		REGISTER_OP_TYPE( Util::Reshape );
		REGISTER_OP_TYPE( Util::Flip );
		REGISTER_OP_TYPE( Util::OnOff );
		REGISTER_OP_TYPE( Util::FlipFlop );
		REGISTER_OP_TYPE( Util::SampleAndHold );
		REGISTER_OP_TYPE( Util::MaxPooling );
		
		//math
		REGISTER_OP_TYPE( Math::AddConst );
		REGISTER_OP_TYPE( Math::MulConst );
		REGISTER_OP_TYPE( Math::Add );
		REGISTER_OP_TYPE( Math::Sub );
		REGISTER_OP_TYPE( Math::Mul );
		REGISTER_OP_TYPE( Math::LinEqConst );
		REGISTER_OP_TYPE( Math::LinEq );
		REGISTER_OP_TYPE( Math::MixConst );
		REGISTER_OP_TYPE( Math::Mix );
		REGISTER_OP_TYPE( Math::Abs );
		REGISTER_OP_TYPE( Math::PowConst );
		REGISTER_OP_TYPE( Math::Sqrt );
		REGISTER_OP_TYPE( Math::Log );
		REGISTER_OP_TYPE( Math::Exp );
		REGISTER_OP_TYPE( Math::ExpConst );
		REGISTER_OP_TYPE( Math::Sigmoid );
		REGISTER_OP_TYPE( Math::Sum );
		REGISTER_OP_TYPE( Math::Product );
		REGISTER_OP_TYPE( Math::Invert );
		REGISTER_OP_TYPE( Math::Slope );
		REGISTER_OP_TYPE( Math::Threshold );
		REGISTER_OP_TYPE( Math::Remap );
		REGISTER_OP_TYPE( Math::Normalize );
		REGISTER_OP_TYPE( Math::AutoNormalize );
		REGISTER_OP_TYPE( Math::MinConst );
		REGISTER_OP_TYPE( Math::Min );
		REGISTER_OP_TYPE( Math::MaxConst );
		REGISTER_OP_TYPE( Math::Max );
		REGISTER_OP_TYPE( Math::Clamp );
		REGISTER_OP_TYPE( Math::ClampConst );
		REGISTER_OP_TYPE( Math::InRange );
		REGISTER_OP_TYPE( Math::Approx );
		REGISTER_OP_TYPE( Math::And );
		REGISTER_OP_TYPE( Math::Or );
		REGISTER_OP_TYPE( Math::XOr );
		REGISTER_OP_TYPE( Math::Determinant );
		REGISTER_OP_TYPE( Math::LinearFunction );
		REGISTER_OP_TYPE( Math::MultiLinearFunction );
		REGISTER_OP_TYPE( Math::FFT2D );
#ifdef __FFTW_SUPPORT
		REGISTER_OP_TYPE( Math::FFT1D );
#endif
		REGISTER_OP_TYPE( Math::ToPolar );

		//pointclouds
		REGISTER_OP_TYPE( PointClouds::ProjectTo3D );

		//temporal
		REGISTER_OP_TYPE( Temporal::RunningAverage );
		REGISTER_OP_TYPE( Temporal::Integral );
		REGISTER_OP_TYPE( Temporal::Drag );
		REGISTER_OP_TYPE( Temporal::BoxFilter );
		REGISTER_OP_TYPE( Temporal::Median );
		REGISTER_OP_TYPE( Temporal::Mean );
		REGISTER_OP_TYPE( Temporal::BGSubtraction );
		REGISTER_OP_TYPE( Temporal::OpticalFlow );
		REGISTER_OP_TYPE( Temporal::Kalman );
		REGISTER_OP_TYPE( Temporal::Resample );

		//statistics
		REGISTER_OP_TYPE( Statistics::CenterOfMass );
		REGISTER_OP_TYPE( Statistics::Histogram );

		//tracking
		REGISTER_OP_TYPE( Tracking::BlobTracker );

		//color
		REGISTER_OP_TYPE( Color::ToGrayscale );
		REGISTER_OP_TYPE( Color::Convert );
		REGISTER_OP_TYPE( Color::HSVShift );

		//ocv
		REGISTER_OP_TYPE( OCV::OCVCam );
		REGISTER_OP_TYPE( OCV::OCVVideoIn );
		REGISTER_OP_TYPE( OCV::OCVVideoOut );
		REGISTER_OP_TYPE( OCV::OCVImageIn );
		REGISTER_OP_TYPE( OCV::OCVImageOut );

		//imaging
		REGISTER_OP_TYPE( Imaging::LowPass );
		REGISTER_OP_TYPE( Imaging::HighPass );
		REGISTER_OP_TYPE( Imaging::Blur );
		REGISTER_OP_TYPE( Imaging::Morph );
		//REGISTER_OP_TYPE( Imaging::CameraIntrinsics );
		//REGISTER_OP_TYPE( Imaging::CameraExtrinsics );

		//sensor
		REGISTER_OP_TYPE( Sensor );

		//sink
		REGISTER_OP_TYPE( Sink );

		//serial
		REGISTER_OP_TYPE( Serial::SerialOut );

		//file
		REGISTER_OP_TYPE( File::FileIn );
		REGISTER_OP_TYPE( File::FileOut );

		//osc
		REGISTER_OP_TYPE( OSC::OSCOut );

#ifdef __COMPRESSION_SUPPORT
		//compression
		//REGISTER_OP_TYPE( Util::Compress );
		//REGISTER_OP_TYPE( Util::Decompress );
#endif

		return true;
	}
}