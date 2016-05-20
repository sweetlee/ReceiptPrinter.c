# Validate with `pod lib lint ReceiptPrinter.podspec`

Pod::Spec.new do |s|
  s.name             = "ReceiptPrinter"
  s.version          = "0.1.1"
  s.summary          = "ReceiptPrinter"
  s.description      = "Receipt Printer"
  s.homepage         = "https://github.com/xslim/ReceiptPrinter"
  s.license          = 'MIT'
  s.author           = { "Taras Kalapun" => "t.kalapun@gmail.com" }
  s.source           = { :git => "https://github.com/xslim/zint.c.git", :tag => s.version.to_s }


  s.platform     = :ios, '6.0'
  s.requires_arc = true

  s.source_files = 'ReceiptPrinter/*.{h,m}', 'printer/*.{h,c}', 'bytebuf/*.{h,c}'
  s.header_mappings_dir = '*'

  # s.public_header_files = 'ReceiptPrinter/*.h'
  s.frameworks = 'ExternalAccessory'

  # s.default_subspecs = 'bytebuf', 'printer'

  # subspec "printer" do |sp|
  #   sp.source_files = 'printer/*.{h,c}'
  # end
  #
  # subspec "bytebuf" do |sp|
  #   sp.source_files = 'bytebuf/*.{h,c}'
  # end

end
