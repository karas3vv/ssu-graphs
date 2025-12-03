#import "conf.typ": conf, intro, conclusion
#show: conf.with(
  title: [Отчет по теории графов],
  type: "referat",
  info: (
    author: (
      name: [Карасева Вадима Дмитриевича],
      faculty: [КНиИТ],
      group: "351",
      sex: "male",
    ),
    inspector: (
      degree: "доцент, к. ф.-м. н.",
      name: "С. В. Миронов",
    ),
  ),
  settings: (
    title_page: (
      enabled: true,
    ),
    contents_page: (
      enabled: true,
    ),
  ),
)

//#intro
//#conclusion
#for value in ("1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11") {
  include "sections/" + value + ".typ"
}