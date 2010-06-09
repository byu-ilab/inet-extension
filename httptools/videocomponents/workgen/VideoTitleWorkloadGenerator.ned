//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package  inet.httptools.videocomponents.workgen;

simple VideoTitleWorkloadGenerator
{
    parameters:
        @display("i=block/cogwheel");
        
        // I/O directories
        	// If this is NOT empty the workload generator will read in the meta data
        	// configuration from the indicated file, the associated .vtmd
        	// files should be in the same directory as the vtwg.cfg file.
        	// If it is empty then the	workload generator will generate
        	// a new set of video titles per the configuration options
        	// below.
        string inputFile = default("");

			// If is not empty the generated video titles will be output to
			// the indicated directory.
        string outputDir = default("");

        // meta data variables
        //int numVODTitles = default(1000);
        //int numLiveTitles = default(120);
        	// currently there is no difference between VOD and Live
        int numTitles = default(1120);
        volatile double titleDuration @unit(min) = default(replaceUnit( uniform(30, 120), "min") );
        double segmentDuration @unit(s) = default(2s);
        int numBitRates = default(12);
        	// bit rate level * quality interval = 2 sec segment size at that bit rate	
        int qualityInterval @unit(KB) = default(35KB);

        // popularity variables
        double zipfExponent = default(2.0);
        double zipfRankOffset = default(2000);
        
        // for testing
        	// leave at 0 (or negative) if you do not want to run tests
        int requests = default(0);
}
