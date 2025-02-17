/**
 *  Copyright 2014 by Benjamin Land (a.k.a. BenLand100)
 *
 *  WbLSdaq is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  WbLSdaq is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with WbLSdaq. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <cmath>
#include <iostream>
#include <stdexcept>
 
#include "V1730.hh"

using namespace std;

V1730Settings::V1730Settings() : DigitizerSettings("") {
    //These are "do nothing" defaults
    card.dual_trace = 0; // 1 bit
    card.analog_probe = 0; // 2 bit (see docs)
    card.oscilloscope_mode = 1; // 1 bit
    card.digital_virt_probe_1 = 0; // 3 bit (see docs)
    card.digital_virt_probe_2 = 0; // 3 bit (see docs)
    card.coincidence_window = 1; // 3 bit
    card.global_majority_level = 0; // 3 bit
    card.external_global_trigger = 0; // 1 bit
    card.software_global_trigger = 0; // 1 bit
    card.out_logic = 0; // 2 bit (OR,AND,MAJORITY)
    card.out_majority_level = 0; // 3 bit
    card.external_trg_out = 0; // 1 bit
    card.software_trg_out = 0; // 1 bit
    card.max_board_agg_blt = 5;
    
    for (uint32_t ch = 0; ch < 16; ch++) {
        chanDefaults(ch);
    }
    
}

V1730Settings::V1730Settings(RunTable &digitizer, RunDB &db) : DigitizerSettings(digitizer.getIndex()){
    
    card.dual_trace = 0; // 1 bit
    card.analog_probe = 0; // 2 bit (see docs)
    card.oscilloscope_mode = 1; // 1 bit
    card.digital_virt_probe_1 = 0; // 3 bit (see docs)
    card.digital_virt_probe_2 = 0; // 3 bit (see docs)
    
    card.coincidence_window = digitizer["coincidence_window"].cast<int>(); // 3 bit
    card.global_majority_level = digitizer["global_majority_level"].cast<int>(); // 3 bit
    card.external_global_trigger = digitizer["external_trigger_enable"].cast<bool>() ? 1 : 0; // 1 bit
    card.software_global_trigger = digitizer["software_trigger_enable"].cast<bool>() ? 1 : 0; // 1 bit
    card.out_logic = digitizer["trig_out_logic"].cast<int>(); // 2 bit (OR,AND,MAJORITY)
    card.out_majority_level = digitizer["trig_out_majority_level"].cast<int>(); // 3 bit
    card.external_trg_out = digitizer["external_trigger_out"].cast<bool>() ? 1 : 0; // 1 bit
    card.software_trg_out = digitizer["external_trigger_out"].cast<bool>() ? 1 : 0; // 1 bit
    card.max_board_agg_blt = digitizer["aggregates_per_transfer"].cast<int>(); 
    
    for (int ch = 0; ch < 16; ch++) {
        if (ch%2 == 0) {
            string grname = "GR"+to_string(ch/2);
            if (!db.tableExists(grname,index)) {
                groupDefaults(ch/2);
            } else {
                cout << "\t" << grname << endl;
                RunTable group = db.getTable(grname,index);
                
                groups[ch/2].local_logic = group["local_logic"].cast<int>(); // 2 bit (see docs)
                groups[ch/2].valid_logic = group["valid_logic"].cast<int>(); // 2 bit (see docs)
                groups[ch/2].global_trigger = group["request_global_trigger"].cast<bool>() ? 1 : 0; // 1 bit
                groups[ch/2].trg_out = group["request_trig_out"].cast<bool>() ? 1 : 0; // 1 bit
                groups[ch/2].valid_mask = 0;
                vector<bool> mask = group["validation_mask"].toVector<bool>();
                for (size_t i = 0; i < mask.size(); i++) {
                    groups[ch/2].valid_mask |= (mask[i] ? 1 : 0) << i;
                }
                groups[ch/2].valid_mode = group["validation_mode"].cast<int>(); // 2 bit
                groups[ch/2].valid_majority = group["validation_majority_level"].cast<int>(); // 3 bit
                groups[ch/2].record_length = group["record_length"].cast<int>(); // 16* bit
                groups[ch/2].ev_per_buffer = group["events_per_buffer"].cast<int>(); // 10 bit
            
            }
        }
        string chname = "CH"+to_string(ch);
        if (!db.tableExists(chname,index)) {
            chanDefaults(ch);
        } else {
            cout << "\t" << chname << endl;
            RunTable channel = db.getTable(chname,index);
    
            //chans[ch].dynamic_range = 0; // 1 bit
            chans[ch].fixed_baseline = 0; // 12 bit
            
            chans[ch].enabled = channel["enabled"].cast<bool>() ? 1 : 0; //1 bit
            chans[ch].dc_offset = round((-channel["dc_offset"].cast<double>()+1.0)/2.0*pow(2.0,16.0)); // 16 bit (-1V to 1V)
            chans[ch].baseline_mean = channel["baseline_average"].cast<int>(); // 3 bit (fixed,16,64,256,1024)
            chans[ch].pulse_polarity = channel["pulse_polarity"].cast<int>(); // 1 bit (0->positive, 1->negative)
            chans[ch].trg_threshold =  channel["trigger_threshold"].cast<int>();// 12 bit
            chans[ch].self_trigger = channel["self_trigger"].cast<bool>() ? 0 : 1; // 1 bit (0->enabled, 1->disabled)
            chans[ch].pre_trigger = channel["pre_trigger"].cast<int>(); // 9* bit
            chans[ch].gate_offset = channel["gate_offset"].cast<int>(); // 8 bit
            chans[ch].long_gate = channel["long_gate"].cast<int>(); // 12 bit
            chans[ch].short_gate = channel["short_gate"].cast<int>(); // 12 bit
            chans[ch].charge_sensitivity = channel["charge_sensitivity"].cast<int>(); // 3 bit (see docs)
            chans[ch].shaped_trigger_width = channel["shaped_trigger_width"].cast<int>(); // 10 bit
            chans[ch].trigger_holdoff = channel["trigger_holdoff"].cast<int>(); // 10* bit
            chans[ch].trigger_config = channel["trigger_type"].cast<int>(); // 2 bit (see docs)
	    chans[ch].dynamic_range = channel["dynamic_range"].cast<int>(); // 1 bit (see docs 0->2Vpp, 1->0.5Vpp)
        }
    }
}

V1730Settings::~V1730Settings() {

}
        
void V1730Settings::validate() { //FIXME validate bit fields too
    for (int ch = 0; ch < 16; ch++) {
        if (ch % 2 == 0) {
            if (groups[ch/2].record_length > 65535) throw runtime_error("Number of samples exceeds 65535 (gr " + to_string(ch/2) + ")");
            if (groups[ch/2].ev_per_buffer < 2) throw runtime_error("Number of events per channel buffer must be at least 2 (gr " + to_string(ch/2) + ")");
            if (groups[ch/2].ev_per_buffer > 1023) throw runtime_error("Number of events per channel buffer exceeds 1023 (gr " + to_string(ch/2) + ")");
        }
        if (chans[ch].pre_trigger > 2044) throw runtime_error("Pre trigger samples exceeds 2044 (ch " + to_string(ch) + ")");
        if (chans[ch].short_gate > 4095) throw runtime_error("Short gate samples exceeds 4095 (ch " + to_string(ch) + ")");
        if (chans[ch].long_gate > 4095) throw runtime_error("Long gate samples exceeds 4095 (ch " + to_string(ch) + ")");
        if (chans[ch].gate_offset > 255) throw runtime_error("Gate offset samples exceeds 255 (ch " + to_string(ch) + ")");
        if (chans[ch].pre_trigger < chans[ch].gate_offset + 19) throw runtime_error("Gate offset and pre_trigger relationship violated (ch " + to_string(ch) + ")");
        if (chans[ch].trg_threshold > 4095) throw runtime_error("Trigger threshold exceeds 4095 (ch " + to_string(ch) + ")");
        if (chans[ch].fixed_baseline > 4095) throw runtime_error("Fixed baseline exceeds 4095 (ch " + to_string(ch) + ")");
        if (chans[ch].shaped_trigger_width > 1023) throw runtime_error("Shaped trigger width exceeds 1023 (ch " + to_string(ch) + ")");
        if (chans[ch].trigger_holdoff > 4092) throw runtime_error("Trigger holdoff width exceeds 4092 (ch " + to_string(ch) + ")");
        if (chans[ch].dc_offset > 65535) throw runtime_error("DC Offset cannot exceed 65535 (ch " + to_string(ch) + ")");
    }
}

void V1730Settings::chanDefaults(uint32_t ch) {
    chans[ch].enabled = 0; //1 bit
    chans[ch].dynamic_range = 0; // 1 bit
    chans[ch].pre_trigger = 30; // 9* bit
    chans[ch].long_gate = 20; // 12 bit
    chans[ch].short_gate = 10; // 12 bit
    chans[ch].gate_offset = 5; // 8 bit
    chans[ch].trg_threshold = 100; // 12 bit
    chans[ch].fixed_baseline = 0; // 12 bit
    chans[ch].shaped_trigger_width = 10; // 10 bit
    chans[ch].trigger_holdoff = 30; // 10* bit
    chans[ch].charge_sensitivity = 000; // 3 bit (see docs)
    chans[ch].pulse_polarity = 1; // 1 bit (0->positive, 1->negative)
    chans[ch].trigger_config = 0; // 2 bit (see docs)
    chans[ch].baseline_mean = 3; // 3 bit (fixed,16,64,256,1024)
    chans[ch].self_trigger = 1; // 1 bit (0->enabled, 1->disabled)
    chans[ch].dc_offset = 0x8000; // 16 bit (-1V to 1V)
}

void V1730Settings::groupDefaults(uint32_t gr) {
    groups[gr].local_logic = 3; // 2 bit
    groups[gr].valid_logic = 3; // 2 bit
    groups[gr].global_trigger = 0; // 1 bit
    groups[gr].trg_out = 0; // 1 bit
    groups[gr].record_length = 200; // 16* bit
    groups[gr].ev_per_buffer = 50; // 10 bit
}

V1730::V1730(VMEBridge &_bridge, uint32_t _baseaddr) : Digitizer(_bridge,_baseaddr) {

}

V1730::~V1730() {
    //Fully reset the board just in case
    write32(REG_BOARD_CONFIGURATION_RELOAD,0);
}

void V1730::calib() {
    write32(REG_CHANNEL_CALIB,0xAAAAAAAA);
}

bool V1730::program(DigitizerSettings &_settings) {
    V1730Settings &settings = dynamic_cast<V1730Settings&>(_settings);
    try {
        settings.validate();
    } catch (runtime_error &e) {
        cout << "Could not program V1730: " << e.what() << endl;
        return false;
    }

    //used to build bit fields
    uint32_t data;
    
    //Fully reset the board just in case
    write32(REG_BOARD_CONFIGURATION_RELOAD,0);
    
    //Front panel config
    data = (1<<0) //ttl
         | (0<<2) | (0<<3) | (0<<4) | (0<<5) //LVDS all input
         | (2<<6) // pattern mode
         | (0<<8) // old lvds features
         | (0<<9);// latch on internal trigger 
    write32(REG_FRONT_PANEL_CONTROL,data);
    
    //LVDS new features config
    data = (0<<0) | (0<<4) | (0<<8) | (0<<12); // ignored for now
    write32(REG_LVDS_NEW_FEATURES,data);

    data = (1 << 2) //individual trigger propagation
         | (1 << 4) //reserved
         | (1 << 8) //individual trigger (reserved)
         | (settings.card.dual_trace << 11) 
         | (settings.card.analog_probe << 12) 
         | (settings.card.oscilloscope_mode << 16) 
         | (1 << 17) //enable extras
         | (1 << 18) //time stamp (reserved)
         | (1 << 19) //charge record (reserved)
         | (settings.card.digital_virt_probe_1 << 23)
         | (settings.card.digital_virt_probe_2 << 26);
    write32(REG_CONFIG,data);

    //build masks while configuring channels
    uint32_t channel_enable_mask = 0;
    uint32_t global_trigger_mask = (settings.card.coincidence_window << 20)
                                 | (settings.card.global_majority_level << 24) 
                                 | (settings.card.external_global_trigger << 30) 
                                 | (settings.card.software_global_trigger << 31);
    uint32_t trigger_out_mask = (settings.card.out_logic << 8) 
                              | (settings.card.out_majority_level << 10)
                              | (settings.card.external_trg_out << 30) 
                              | (settings.card.software_trg_out << 31);
    
    //keep track of the size of the buffers for each memory location
    uint32_t buffer_sizes[8] = { 0, 0, 0, 0, 0, 0, 0, 0}; //in locations

    for (int ch = 0; ch < 16; ch++) {
        channel_enable_mask |= (settings.chans[ch].enabled << ch);
        
        if (ch % 2 == 0) {
            global_trigger_mask |= (settings.groups[ch/2].global_trigger << (ch/2));
            trigger_out_mask |= (settings.groups[ch/2].trg_out << (ch/2));
            
            data = settings.groups[ch/2].record_length%8 ?  settings.groups[ch/2].record_length/8+1 : settings.groups[ch/2].record_length/8;
            write32(REG_RECORD_LENGTH|(ch<<8),data);
            settings.groups[ch/2].record_length = read32(REG_RECORD_LENGTH|(ch<<8))*8;
            
            data = settings.groups[ch/2].valid_mask
                 | (settings.groups[ch/2].valid_mode << 8)
                 | (settings.groups[ch/2].valid_majority << 10);
            write32(REG_LOCAL_VALIDATION+(ch/2*4),data);
        } else {
            write32(REG_RECORD_LENGTH|(ch<<8),settings.groups[ch/2].record_length/8);
        }
        
        if (settings.chans[ch].enabled) {
            buffer_sizes[ch/2] = (2 + settings.groups[ch/2].record_length/8)*settings.groups[ch/2].ev_per_buffer;
        }
        
        write32(REG_NEV_AGGREGATE|(ch<<8),settings.groups[ch/2].ev_per_buffer);
        write32(REG_PRE_TRG|(ch<<8),settings.chans[ch].pre_trigger/4);
        //write32(REG_SHORT_GATE|(ch<<8),settings.chans[ch].short_gate);
        //write32(REG_LONG_GATE|(ch<<8),settings.chans[ch].long_gate);
        //write32(REG_PRE_GATE|(ch<<8),settings.chans[ch].gate_offset);
        //write32(REG_DPP_TRG_THRESHOLD|(ch<<8),settings.chans[ch].trg_threshold);
        write32(REG_BASELINE_THRESHOLD|(ch<<8),settings.chans[ch].fixed_baseline);
        write32(REG_SHAPED_TRIGGER_WIDTH|(ch<<8),settings.chans[ch].shaped_trigger_width);
        write32(REG_TRIGGER_HOLDOFF|(ch<<8),settings.chans[ch].trigger_holdoff/4);
        //data = (settings.chans[ch].charge_sensitivity)
        //     | (settings.chans[ch].pulse_polarity << 16)
        //     | (settings.chans[ch].trigger_config << 18)
        //     | (settings.chans[ch].baseline_mean << 20)
        //     | (settings.chans[ch].self_trigger << 24);
        //write32(REG_DPP_CTRL|(ch<<8),data);
        data = (settings.groups[ch/2].local_logic<<0) 
             | (1<<2) // enable request logic
             | (settings.groups[ch/2].valid_logic<<4)
             | (1<<6); // enable valid logic
        write32(REG_TRIGGER_CTRL|(ch<<8),data);
        write32(REG_DC_OFFSET|(ch<<8),settings.chans[ch].dc_offset);
        
    }
    
    write32(REG_CHANNEL_ENABLE_MASK,channel_enable_mask);
    write32(REG_GLOBAL_TRIGGER_MASK,global_trigger_mask);
    write32(REG_TRIGGER_OUT_MASK,trigger_out_mask);
    
    uint32_t largest_buffer = 0;
    for (int i = 0; i < 8; i++) if (largest_buffer < buffer_sizes[i]) largest_buffer = buffer_sizes[i];
    const uint32_t total_locations = 640000/8; 
    const uint32_t num_buffers = total_locations%largest_buffer ? total_locations/largest_buffer : total_locations/largest_buffer+1;
    uint32_t shifter = num_buffers;
    for (settings.card.buff_org = 0; shifter != 1; shifter = shifter >> 1, settings.card.buff_org++);
    if (1u << settings.card.buff_org > num_buffers) settings.card.buff_org--;
    if (settings.card.buff_org > 0xA) settings.card.buff_org = 0xA;
    if (settings.card.buff_org < 0x2) settings.card.buff_org = 0x2;
    cout << "Largest buffer: " << largest_buffer << " loc\nDesired buffers: " << num_buffers << "\nProgrammed buffers: " << (1<<settings.card.buff_org) << endl;
    write32(REG_BUFF_ORG,settings.card.buff_org);
    
    //Set max board aggregates to transver per readout
    write16(REG_READOUT_BLT_AGGREGATE_NUMBER,settings.card.max_board_agg_blt);
    
    //Enable VME BLT readout
    write16(REG_READOUT_CONTROL,1<<4);
    
    return true;
}

void V1730::softTrig() {
    write32(REG_SOFTWARE_TRIGGER,0xDEADBEEF);
}

void V1730::startAcquisition() {
    write32(REG_ACQUISITION_CONTROL,1<<2);
}

void V1730::stopAcquisition() {
    write32(REG_ACQUISITION_CONTROL,0);
}

bool V1730::acquisitionRunning() {
    return read32(REG_ACQUISITION_STATUS) & (1 << 2);
}

bool V1730::readoutReady() {
    return read32(REG_ACQUISITION_STATUS) & (1 << 3);
}


bool V1730::checkTemps(vector<uint32_t> &temps, uint32_t danger) {
    temps.resize(16);
    bool over = false;
    for (int ch = 0; ch < 16; ch++) {
        temps[ch] = read32(REG_CHANNEL_TEMP|(ch<<8));
        if (temps[ch] >= danger) over = true;
    }
    return over;
}

size_t V1730::readoutBLT_evtsz(char *buffer, size_t buffer_size) {
    size_t offset = 0, total = 0;
    while (readoutReady()) {
        uint32_t next = read32(REG_EVENT_SIZE);
        total += 4*next;
        if (offset+total > buffer_size) throw runtime_error("Readout buffer too small!");
        size_t lastoff = offset;
        while (offset < total) {
            size_t remaining = total-offset, read;
            if (remaining > 4090) {
                read = readBLT(0x0000, buffer+offset, 4090);
            } else {
                remaining = 8*(remaining%8 ? remaining/8+1 : remaining/8); // needs to be multiples of 8 (64bit)
                read = readBLT(0x0000, buffer+offset, remaining);
            }
            offset += read;
            if (!read) {
                cout << "\tfailed event size " << offset-lastoff << " / " << next*4 << endl;
                break;
            }
        }
    }
    return total;
}



V1730Decoder::V1730Decoder(size_t _eventBuffer, V1730Settings &_settings) : eventBuffer(_eventBuffer), settings(_settings) {

    dispatch_index = decode_counter = chanagg_counter = boardagg_counter = 0;
    
    for (size_t ch = 0; ch < 16; ch++) {
        if (settings.getEnabled(ch)) {
            chan2idx[ch] = nsamples.size();
            idx2chan[nsamples.size()] = ch;
            nsamples.push_back(settings.getRecordLength(ch));
            grabbed.push_back(0);
            if (eventBuffer > 0) {
                grabs.push_back(new uint16_t[eventBuffer*nsamples.back()]);
                patterns.push_back(new uint16_t[eventBuffer]);
                baselines.push_back(new uint16_t[eventBuffer]);
                qshorts.push_back(new uint16_t[eventBuffer]);
                qlongs.push_back(new uint16_t[eventBuffer]);
                times.push_back(new uint64_t[eventBuffer]);
            }
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC,&last_decode_time);

}

V1730Decoder::~V1730Decoder() {
    for (size_t i = 0; i < grabs.size(); i++) {
        delete [] grabs[i];
        delete [] patterns[i];
        delete [] baselines[i];
        delete [] qshorts[i];
        delete [] qlongs[i];
        delete [] times[i];
    }
}

void V1730Decoder::decode(Buffer &buf) {
    vector<size_t> lastgrabbed(grabbed);
    
    decode_size = buf.fill();
    cout << settings.getIndex() << " decoding " << decode_size << " bytes." << endl;
    uint32_t *next = (uint32_t*)buf.rptr(), *start = (uint32_t*)buf.rptr();
    while ((size_t)((next = decode_board_agg(next)) - start + 1)*4 < decode_size);
    buf.dec(decode_size);
    decode_counter++;
    
    struct timespec cur_time;
    clock_gettime(CLOCK_MONOTONIC,&cur_time);
    double time_int = (cur_time.tv_sec - last_decode_time.tv_sec)+1e-9*(cur_time.tv_nsec - last_decode_time.tv_nsec);
    last_decode_time = cur_time;
    
    for (size_t i = 0; i < idx2chan.size(); i++) {
        cout << "\tch" << idx2chan[i] << "\tev: " << grabbed[i]-lastgrabbed[i] << " / " << (grabbed[i]-lastgrabbed[i])/time_int << " Hz / " << grabbed[i] << " total" << endl;
    }
}

size_t V1730Decoder::eventsReady() {
    size_t grabs = grabbed[0];
    for (size_t idx = 1; idx < grabbed.size(); idx++) {
        if (grabbed[idx] < grabs) grabs = grabbed[idx];
    }
    return grabs;
}

// length, lvdsidx, dsize, nsamples, samples[], strlen, strname[]

void V1730Decoder::dispatch(int nfd, int *fds) {
    
    size_t ready = eventsReady();
    
    for ( ; dispatch_index < ready; dispatch_index++) {
        for (size_t i = 0; i < nsamples.size(); i++) {
            uint8_t lvdsidx = patterns[i][dispatch_index] & 0xFF; 
            uint8_t dsize = 2;
            uint16_t nsamps = nsamples[i];
            uint16_t *samples = &grabs[i][nsamps*dispatch_index];
            string strname = "/"+settings.getIndex()+"/ch" + to_string(idx2chan[i]);
            uint16_t strlen = strname.length();
            uint16_t length = 2+strlen+2+nsamps*2+1+1;
            for (int j = 0; j < nfd; j++) {
                writeall(fds[j],&length,2);
                writeall(fds[j],&lvdsidx,1);
                writeall(fds[j],&dsize,1);
                writeall(fds[j],&nsamps,2);
                writeall(fds[j],samples,nsamps*2);
                writeall(fds[j],&strlen,2);
                writeall(fds[j],strname.c_str(),strlen);
            }
        }
    }
}

using namespace H5;

void V1730Decoder::writeOut(H5File &file, size_t nEvents) {

    cout << "\t/" << settings.getIndex() << endl;

    Group cardgroup = file.createGroup("/"+settings.getIndex());
        
    DataSpace scalar(0,NULL);
    
    double dval;
    uint32_t ival;
    
    Attribute bits = cardgroup.createAttribute("bits",PredType::NATIVE_UINT32,scalar);
    ival = 14;
    bits.write(PredType::NATIVE_INT32,&ival);
    
    Attribute ns_sample = cardgroup.createAttribute("ns_sample",PredType::NATIVE_DOUBLE,scalar);
    dval = 2.0;
    ns_sample.write(PredType::NATIVE_DOUBLE,&dval);
    
    for (size_t i = 0; i < nsamples.size(); i++) {
    
        string chname = "ch" + to_string(idx2chan[i]);
        Group group = cardgroup.createGroup(chname);
        string groupname = "/"+settings.getIndex()+"/"+chname;
        
        cout << "\t" << groupname << endl;
        
        Attribute offset = group.createAttribute("offset",PredType::NATIVE_UINT32,scalar);
        ival = settings.getDCOffset(idx2chan[i]);
        offset.write(PredType::NATIVE_UINT32,&ival);
        
        Attribute samples = group.createAttribute("samples",PredType::NATIVE_UINT32,scalar);
        ival = settings.getRecordLength(idx2chan[i]);
        samples.write(PredType::NATIVE_UINT32,&ival);
        
        Attribute presamples = group.createAttribute("presamples",PredType::NATIVE_UINT32,scalar);
        ival = settings.getPreSamples(idx2chan[i]);
        presamples.write(PredType::NATIVE_UINT32,&ival);
        
        Attribute threshold = group.createAttribute("threshold",PredType::NATIVE_UINT32,scalar);
        ival = settings.getThreshold(idx2chan[i]);
        threshold.write(PredType::NATIVE_UINT32,&ival);
        
        hsize_t dimensions[2];
        dimensions[0] = nEvents;
        dimensions[1] = nsamples[i];
        
        DataSpace samplespace(2, dimensions);
        DataSpace metaspace(1, dimensions);
        
        cout << "\t" << groupname << "/samples" << endl;
        DataSet samples_ds = file.createDataSet(groupname+"/samples", PredType::NATIVE_UINT16, samplespace);
        samples_ds.write(grabs[i], PredType::NATIVE_UINT16);
        memmove(grabs[i],grabs[i]+nEvents*nsamples[i],nsamples[i]*sizeof(uint16_t)*(grabbed[i]-nEvents));
        
        cout << "\t" << groupname << "/patterns" << endl;
        DataSet patterns_ds = file.createDataSet(groupname+"/patterns", PredType::NATIVE_UINT16, metaspace);
        patterns_ds.write(patterns[i], PredType::NATIVE_UINT16);
        memmove(patterns[i],patterns[i]+nEvents,sizeof(uint16_t)*(grabbed[i]-nEvents));
        
        cout << "\t" << groupname << "/baselines" << endl;
        DataSet baselines_ds = file.createDataSet(groupname+"/baselines", PredType::NATIVE_UINT16, metaspace);
        baselines_ds.write(baselines[i], PredType::NATIVE_UINT16);
        memmove(baselines[i],baselines[i]+nEvents,sizeof(uint16_t)*(grabbed[i]-nEvents));
        
        cout << "\t" << groupname << "/qshorts" << endl;
        DataSet qshorts_ds = file.createDataSet(groupname+"/qshorts", PredType::NATIVE_UINT16, metaspace);
        qshorts_ds.write(qshorts[i], PredType::NATIVE_UINT16);
        memmove(qshorts[i],qshorts[i]+nEvents,sizeof(uint16_t)*(grabbed[i]-nEvents));
        
        cout << "\t" << groupname << "/qlongs" << endl;
        DataSet qlongs_ds = file.createDataSet(groupname+"/qlongs", PredType::NATIVE_UINT16, metaspace);
        qlongs_ds.write(qlongs[i], PredType::NATIVE_UINT16);
        memmove(qlongs[i],qlongs[i]+nEvents,sizeof(uint16_t)*(grabbed[i]-nEvents));

        cout << "\t" << groupname << "/times" << endl;
        DataSet times_ds = file.createDataSet(groupname+"/times", PredType::NATIVE_UINT64, metaspace);
        times_ds.write(times[i], PredType::NATIVE_UINT64);
        memmove(times[i],times[i]+nEvents,sizeof(uint64_t)*(grabbed[i]-nEvents));
        
        grabbed[i] -= nEvents;
    }
    
    dispatch_index -= nEvents;
    if (dispatch_index < 0) dispatch_index = 0;
}

uint32_t* V1730Decoder::decode_chan_agg(uint32_t *chanagg, uint32_t group, uint16_t pattern) {
    const bool format_flag = chanagg[0] & 0x80000000;
    if (!format_flag) throw runtime_error("Channel format not found");
    
    chanagg_counter++;
    
    const uint32_t size = chanagg[0] & 0x7FFF;
    const uint32_t format = chanagg[1];
    const uint32_t samples = (format & 0xFFF)*8;
    
    /*
    //Metadata
    const bool dualtrace_enable = format & (1<<31);
    const bool charge_enable =format & (1<<30);
    const bool time_enable = format & (1<<29);
    const bool baseline_enable = format & (1<<28);
    const bool waveform_enable = format & (1<<27);
    const uint32_t extras = (format >> 24) & 0x7;
    const uint32_t analog_probe = (format >> 22) & 0x3;
    const uint32_t digital_probe_2 = (format >> 19) & 0x7;
    const uint32_t digital_probe_1 = (format >> 16) & 0x7;
    */
    
    const uint32_t idx0 = chan2idx.count(group * 2 + 0) ? chan2idx[group * 2 + 0] : 999;
    const uint32_t idx1 = chan2idx.count(group * 2 + 1) ? chan2idx[group * 2 + 1] : 999;
    
    for (uint32_t *event = chanagg+2; (size_t)(event-chanagg+1) < size; event += samples/2+3) {
        
        const bool oddch = event[0] & 0x80000000;
        const uint32_t idx = oddch ? idx1 : idx0;
        const uint32_t len = nsamples[idx];
        
        if (idx == 999) throw runtime_error("Received data for disabled channel (" + to_string(group*2+oddch?1:0) + ")");
        
        if (len != samples) throw runtime_error("Number of samples received " + to_string(samples) + " does not match expected " + to_string(len) + " (" + to_string(idx2chan[idx]) + ")");
        
        if (eventBuffer) {
            const size_t ev = grabbed[idx]++;
            if (ev == eventBuffer) throw runtime_error("Decoder buffer for " + settings.getIndex() + " overflowed!");
            uint16_t *data = grabs[idx] + ev*len;
            
            for (uint32_t *word = event+1, sample = 0; sample < len; word++, sample+=2) {
                data[sample+0] = *word & 0x3FFF;
                //uint8_t dp10 = (*word >> 14) & 0x1;
                //uint8_t dp20 = (*word >> 15) & 0x1;
                data[sample+1] = (*word >> 16) & 0x3FFF;
                //uint8_t dp11 = (*word >> 30) & 0x1;
                //uint8_t dp21 = (*word >> 31) & 0x1;
            }
            
            patterns[idx][ev] = pattern;
            baselines[idx][ev] = event[1+samples/2+0] & 0xFFFF;
            qshorts[idx][ev] = event[1+samples/2+1] & 0x7FFF;
            qlongs[idx][ev] = (event[1+samples/2+1] >> 16) & 0xFFFF;
            times[idx][ev] = ((uint64_t)(event[0] & 0x7FFFFFFF)) | (((uint64_t)(event[1+samples/2+0]&0xFFFF0000))<<15);
        } else {
            grabbed[idx]++;
        }
    
    }
    
    return chanagg + size;
}

uint32_t* V1730Decoder::decode_board_agg(uint32_t *boardagg) {
    if (boardagg[0] == 0xFFFFFFFF) {
        boardagg++; //sometimes padded
    }
    if ((boardagg[0] & 0xF0000000) != 0xA0000000) 
        throw runtime_error("Board aggregate missing tag");
    
    boardagg_counter++;    
    
    uint32_t size = boardagg[0] & 0x0FFFFFFF;
    
    //const uint32_t board = (boardagg[1] >> 28) & 0xF;
    //const bool fail = boardagg[1] & (1 << 26);
    const uint16_t pattern = (boardagg[1] >> 8) & 0x7FFF;
    const uint32_t mask = boardagg[1] & 0xFF;
    
    cout << "\t(LVDS & 0xFF): " << (pattern & 0xFF) << endl;
    
    //const uint32_t count = boardagg[2] & 0x7FFFFF;
    //const uint32_t timetag = boardagg[3];
    
    uint32_t *chans = boardagg+4;
    
    for (uint32_t gr = 0; gr < 8; gr++) {
        if (mask & (1 << gr)) {
            chans = decode_chan_agg(chans,gr,pattern);
        }
    } 
    
    return boardagg+size;
}
