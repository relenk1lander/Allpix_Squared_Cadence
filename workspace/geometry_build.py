chip_type = "faser"

asic_size_y = 15.280
asic_size_x = 22.150

number_of_asics_y = 12
number_of_asics_x = 6
number_of_layers = 6

tungsten_absorber_x = 200
tungsten_absorber_y = 200
tungsten_absorber_z = 4.6

offset_x = asic_size_x * (number_of_asics_x - 1) / 2
offset_y = asic_size_y * (number_of_asics_y - 1) / 2

for layer in range(number_of_layers):
    print('[tungsten_%s]' % layer)
    print('type = "box"')
    print('size = %gmm %gmm %gmm' % (tungsten_absorber_x, tungsten_absorber_y, tungsten_absorber_z))
    print('position = 0mm 0mm %dmm' % (layer * 30.0 - 15.000))
    print('orientation = 0 0 0')
    print('material = tungsten')
    print('role = "passive"\n')
    
    for asic_x in range(number_of_asics_x):
        for asic_y in range(number_of_asics_y):
            print('[Detector_%d_%d_%d]' % (layer + 1, asic_x + 1, asic_y + 1))
            print('type = "%s"' % chip_type)
            pos_x = asic_x * asic_size_x - offset_x 
            pos_y = asic_y * asic_size_y - offset_y - (0.46 if asic_x % 2 else 0)
            print('position = %gmm %gmm %gmm' % (pos_x, pos_y, layer * 30.0))
            print('orientation = 0deg 0deg %ddeg\n' % (180 if asic_x % 2 else 0))
